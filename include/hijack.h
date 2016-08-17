#ifndef __HIJACK__
#define __HIJACK__

#define _GNU_SOURCE

#include "distorm.h"
#include <dlfcn.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define TRAMPOLINE_CONTENTS                                                    \
	asm("nop;nop;nop;nop;nop;nop;nop;nop;"                                     \
		"nop;nop;nop;nop;nop;nop;nop;nop;"                                     \
		"nop;nop;nop;nop;nop;nop;nop;nop;"                                     \
		"nop;nop;nop;nop;nop;nop;nop;nop;"                                     \
		"nop;nop;nop;nop;nop;nop;nop;nop;"                                     \
		"nop;nop;nop;nop;nop;nop;nop;nop;");

// on x86_64 the hijack code is 16 bytes long
#define MAX_HIJACK_SIZE 16
// MAX_HIJACK_SIZE == 16 hence there could be up to 16 1 byte instructions, note
// that the last instruction might be any size up to 16 bytes
#define DECODE_MAX_INSTRUCTIONS 16
// 15 ins X 1 Byte + 1 inst X 16 bytes = 31 bytes
#define MAX_OCODE_SIZE 32

#ifdef __x86_64__
#define DECODE_BITS Decode64Bits
#else
#define DECODE_BITS Decode32Bits
#endif

typedef struct sym_hook {
	void *addr;
	int hijack_size;
	unsigned char o_code[MAX_OCODE_SIZE];
	unsigned char n_code[MAX_HIJACK_SIZE];
} sym_hook;

sym_hook malloc_hook;

// NOTE an assumption is made that target is in typical TEXT region
static void disable_wp(void *target, int size) {
	uintptr_t start = (uintptr_t)target & ~0xFFF;
	uintptr_t end = ((uintptr_t)target + size + 4095) & ~0xFFF;
	mprotect((void *)start, end - start, PROT_WRITE | PROT_READ | PROT_EXEC);
}
static void enable_wp(void *target, int size) {
	uintptr_t start = (uintptr_t)target & ~0xFFF;
	uintptr_t end = ((uintptr_t)target + size + 4095) & ~0xFFF;
	mprotect((void *)start, end - start, PROT_EXEC | PROT_READ);
}

void hijack_resume(sym_hook *hook) {
	disable_wp(hook->addr, hook->hijack_size);
	memcpy(hook->addr, hook->n_code, hook->hijack_size);
	enable_wp(hook->addr, hook->hijack_size);
}

void hijack_stop(sym_hook *hook) {
	disable_wp(hook->addr, hook->hijack_size);
	memcpy(hook->addr, hook->o_code, hook->hijack_size);
	enable_wp(hook->addr, hook->hijack_size);
}

static unsigned int dissasm(void *ptr, _DecodedInst *instructions,
							int max_bytes, int max_instructions) {
	unsigned int decodedCount = 0;
	_DecodeResult result =
		distorm_decode(0, ptr, max_bytes, DECODE_BITS, instructions,
					   max_instructions, &decodedCount);
	if (result == DECRES_INPUTERR) {
		printf("Input error\n");
		return 0;
	}
	return decodedCount;
}

static int hook_whole_size(void *target, unsigned int hook_code_size) {
	_DecodedInst decodedInstructions[DECODE_MAX_INSTRUCTIONS];
	unsigned int decodedCount = dissasm(
		target, decodedInstructions, MAX_OCODE_SIZE, DECODE_MAX_INSTRUCTIONS);
	if (decodedCount == 0) {
		exit(-1);
	}
	unsigned int whole_size = 0, i = 0;
	while (whole_size < hook_code_size) {
		if (i > decodedCount) {
			printf("Didn't decode enough !!!\n");
			exit(-1);
		}
		// FIXME not just JMP, there is plenty of other instructions that use
		// relative addressing, call, conditional jumps
		if (strcmp((char *)decodedInstructions[i].mnemonic.p, "JMP") == 0) {
			// cannot provide trampoline if the original function
			// unconditionally jumps
			printf("Original code contains JMP instruction\n");
			return -1;
		}
		whole_size += decodedInstructions[i++].size;
	}
	return whole_size;
}

static void print_assembly(void *target, int bytes) {
	_DecodedInst decodedInstructions[DECODE_MAX_INSTRUCTIONS];
	unsigned int decodedCount =
		dissasm(target, decodedInstructions, bytes, DECODE_MAX_INSTRUCTIONS);
	if (decodedCount == 0) {
		return;
	}

	for (unsigned int i = 0; i < decodedCount; i++) {
		_DecodedInst inst = decodedInstructions[i];
		printf("%s %s -> %s(%d)\n", inst.mnemonic.p, inst.operands.p,
			   inst.instructionHex.p, inst.size);
	}
}

sym_hook hijack_start(void *target, void *new) {
	sym_hook hook;
	hook.addr = target;
	memcpy(hook.o_code, target, MAX_OCODE_SIZE);
	// check if jump needs to be a long jump
	if ((MAX(new, target) - MIN(new, target)) < INT32_MAX) {
		hook.hijack_size = 5;
		// NOTE jmp $relative_addr;
		memcpy(hook.n_code, "\xe9\x00\x00\x00\x00", 5);
		*(uint32_t *)(&hook.n_code[1]) = (uint32_t)(new - target - 5);
	} else if ((uintptr_t) new < (uintptr_t)UINT32_MAX) {
		hook.hijack_size = 6;
		// NOTE push $addr; ret
		memcpy(hook.n_code, "\x68\x00\x00\x00\x00\xc3", 6);
		*(uint32_t *)(&hook.n_code[1]) = (uint32_t) new;
	} else {
		hook.hijack_size = 16;
		/* NOTE
		push rax;
		movabs rax, $addr;
		xchg rax, [rsp];
		ret;
		*/
		memcpy(hook.n_code, "\x50\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00"
							"\x48\x87\x04\x24\xC3",
			   16);
		*(uint64_t *)(&hook.n_code[3]) = (uint64_t) new;
	}
	disable_wp(target, hook.hijack_size);
	memcpy(target, hook.n_code, hook.hijack_size);
	enable_wp(target, hook.hijack_size);
	return hook;
}

int hijack_make_trampoline(sym_hook *hook, void *trampoline) {
	// print_assembly(hook->o_code, 32);
	int whole_size = hook_whole_size(hook->o_code, hook->hijack_size);
	if (whole_size == -1) {
		return -1;
	}
	// printf("Whole size is %d\n", whole_size);
	disable_wp(trampoline, hook->hijack_size);
	memcpy(trampoline, hook->o_code, whole_size);
	enable_wp(trampoline, hook->hijack_size);
	(void)hijack_start(trampoline + whole_size, hook->addr + whole_size);
	// print_assembly(trampoline, 32);
	return 0;
}
#endif

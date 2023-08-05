#include "log.h"
#include "myvm.h"
#include "myvm_vm.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static uint8_t getNextByte(myvm_VM *vm);
static uint32_t build16From8(uint8_t b1, uint8_t b2);
static uint32_t build32From8(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);

#define MYVM_GET_ARG1(x) ((x & 0b11100000) >> 5)
#define MYVM_GET_ARG2(x) ((x & 0b00011100) >> 2)

/* TODO: little endian or big endian? (probably big endian, which actually means
 * 0xAABB is [AA, BB]) */

/* TODO: GLFW UI */

bool myvm_VM_init(myvm_VM *dest, size_t available_memory) {
	/* int size assertions */
	assert(sizeof(uint8_t) == 1);
	assert(sizeof(uint16_t) == 2);
	assert(sizeof(uint32_t) == 4);

	memset(&dest->registers, 0, sizeof(dest->registers));
	dest->registers[MYVM_R_INSPTR] = 0x100;

	uint8_t *mem = malloc(available_memory);
	if (mem == NULL) {
		logD("OOM (allocating VM RAM)");
		return false;
	}

	/* fill it all with 0xFF */
	memset(mem, 0xFF, available_memory);

	dest->memory.ptr = mem;
	dest->memory.len = available_memory;

	return true;
}

void myvm_VM_deinit(myvm_VM *vm) {
	free(vm->memory.ptr);
}

bool myvm_VM_copyRom(myvm_VM *vm, myvm_BufConst rom) {
	if (rom.len > vm->memory.len - 0x100) {
		logD("not enough available memory for copying ROM");
		return false;
	}
	memcpy(&vm->memory.ptr[0x100], rom.ptr, rom.len);
	return true;
}

bool myvm_VM_run(myvm_VM *vm) {
	for (;;) {
		uint32_t insptr = vm->registers[MYVM_R_INSPTR];
		uint8_t ins = getNextByte(vm);
		/* logD("at #%x : ins #%x", insptr, ins); */

		switch (ins) {
		case MYVM_INS_BRK:
			logD("emulation finished");
			return true;

		case MYVM_INS_CPY: {
			uint8_t argflag = getNextByte(vm);
			uint8_t arg1 = MYVM_GET_ARG1(argflag);
			uint8_t arg2 = MYVM_GET_ARG2(argflag);
			logD("cpy r%x <- r%x", arg1, arg2);
			vm->registers[arg1] = vm->registers[arg2];
			break;
		}

		case MYVM_INS_LD8: {
			uint8_t arg1 = MYVM_GET_ARG1(getNextByte(vm));
			uint8_t b1 = getNextByte(vm);
			logD("ld8 r%x <- #%02x", arg1, b1);
			vm->registers[arg1] = b1;
			break;
		}

		case MYVM_INS_LD16: {
			uint8_t arg1 = MYVM_GET_ARG1(getNextByte(vm));
			uint8_t b1 = getNextByte(vm);
			uint8_t b2 = getNextByte(vm);
			logD("ld16 r%x <- #%02x", arg1, build16From8(b1, b2));
			vm->registers[arg1] = build16From8(b1, b2);
			break;
		}

		case MYVM_INS_LD32: {
			uint8_t arg1 = MYVM_GET_ARG1(getNextByte(vm));
			uint8_t b1 = getNextByte(vm);
			uint8_t b2 = getNextByte(vm);
			uint8_t b3 = getNextByte(vm);
			uint8_t b4 = getNextByte(vm);
			logD("ld32 r%x <- #%02x", arg1, build32From8(b1, b2, b3, b4));
			vm->registers[arg1] = build32From8(b1, b2, b3, b4);
			break;
		}

		case MYVM_INS_PRINT: {
			uint8_t arg1 = MYVM_GET_ARG1(getNextByte(vm));
			uint32_t r =vm->registers[arg1];
			logD("r%x is %02d (#%02x)", arg1, r, r);
			break;
		}

		default:
			logD("unknown instruction: %d", ins);
			return false;
		}
	}
}

static uint8_t getNextByte(myvm_VM *vm) {
	size_t idx = (size_t) vm->registers[MYVM_R_INSPTR];
	if (idx >= vm->memory.len) die("instruction pointer out of bounds");
	vm->registers[MYVM_R_INSPTR]++;
	return vm->memory.ptr[idx];
}

static uint32_t build32From8(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
	return ((uint32_t) b1 << 24) + ((uint32_t) b2 << 16) + ((uint32_t) b3 << 8) + (uint32_t) b4;
}

static uint32_t build16From8(uint8_t b1, uint8_t b2) {
	return ((uint32_t) b1 << 8) + (uint32_t) b2;
}

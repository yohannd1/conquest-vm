#include "log.h"
#include "conq.h"
#include "conq_vm.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static uint8_t getNextByte(conq_VM *vm);
static uint32_t build16From8(uint8_t b1, uint8_t b2);
static uint32_t build32From8(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);

#define MYVM_GET_ARG1(x) ((x & 0b11100000) >> 5)
#define MYVM_GET_ARG2(x) ((x & 0b00011100) >> 2)

bool conq_VM_init(conq_VM *dest, size_t available_memory) {
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

void conq_VM_deinit(conq_VM *vm) {
	free(vm->memory.ptr);
}

bool conq_VM_copyRom(conq_VM *vm, conq_BufConst rom) {
	if (rom.len > vm->memory.len - 0x100) {
		logD("not enough available memory for copying ROM");
		return false;
	}
	memcpy(&vm->memory.ptr[0x100], rom.ptr, rom.len);
	return true;
}

bool conq_VM_run(conq_VM *vm) {
	for (;;) {
		uint8_t ins = getNextByte(vm);

		switch (ins) {
		case MYVM_INS_BRK:
			logD("emulation finished");
			return true;

		case MYVM_INS_CPY: {
			uint8_t regs = getNextByte(vm);
			uint8_t rdest = MYVM_GET_ARG1(regs);
			uint8_t rval = MYVM_GET_ARG2(regs);

			logD("cpy r%x <- r%x", rdest, rval);
			vm->registers[rdest] = vm->registers[rval];
			break;
		}

		case MYVM_INS_LD8: {
			uint8_t arg1 = MYVM_GET_ARG1(getNextByte(vm));
			uint8_t b1 = getNextByte(vm);
			logD("ld8 r%x <- %d (#%02x)", arg1, b1, b1);
			vm->registers[arg1] = b1;
			break;
		}

		case MYVM_INS_LD16: {
			uint8_t arg1 = MYVM_GET_ARG1(getNextByte(vm));
			uint8_t b1 = getNextByte(vm);
			uint8_t b2 = getNextByte(vm);
			uint16_t n = build16From8(b1, b2);
			logD("ld16 r%x <- %d (#%02x)", arg1, n, n);
			vm->registers[arg1] = build16From8(b1, b2);
			break;
		}

		case MYVM_INS_LD32: {
			uint8_t arg1 = MYVM_GET_ARG1(getNextByte(vm));
			uint8_t b1 = getNextByte(vm);
			uint8_t b2 = getNextByte(vm);
			uint8_t b3 = getNextByte(vm);
			uint8_t b4 = getNextByte(vm);
			uint32_t n = build32From8(b1, b2, b3, b4);
			logD("ld32 r%x <- %d (#%02x)", arg1, n, n);
			vm->registers[arg1] = n;
			break;
		}

		case MYVM_INS_PRINT: {
			uint8_t arg1 = MYVM_GET_ARG1(getNextByte(vm));
			uint32_t r = vm->registers[arg1];
			logD("r%x is %02d (#%02x)", arg1, r, r);
			break;
		}

		case MYVM_INS_WR8: {
			uint8_t regs = getNextByte(vm);
			uint8_t rdest = MYVM_GET_ARG1(regs);
			uint8_t rval = MYVM_GET_ARG2(regs);

			logD("wr8 *r%x (@#%02x) <- r%x (#%02x)", rdest, vm->registers[rdest], rval, vm->registers[rval]);

			uint32_t dest = vm->registers[rdest];
			if (dest >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx; max memory is #%02lx", dest, vm->memory.len);
				return false;
			}

			uint32_t val = vm->registers[rval];
			vm->memory.ptr[dest] = val;
			break;
		}

		case MYVM_INS_WR16: {
			uint8_t regs = getNextByte(vm);
			uint8_t rdest = MYVM_GET_ARG1(regs);
			uint8_t rval = MYVM_GET_ARG2(regs);

			logD("wr16 *r%x (@#%02x) <- r%x (#%02x)", rdest, vm->registers[rdest], rval, vm->registers[rval]);

			uint32_t dest = vm->registers[rdest];
			if (dest + 1 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx (2 bytes); max memory is #%02lx", dest, vm->memory.len);
				return false;
			}

			uint32_t val = vm->registers[rval];

			uint8_t b1 = (uint8_t)(val >> 8);
			uint8_t b2 = (uint8_t)val;
			vm->memory.ptr[dest] = b1;
			vm->memory.ptr[dest+1] = b2;
			break;
		}

		case MYVM_INS_WR32: {
			uint8_t regs = getNextByte(vm);
			uint8_t rdest = MYVM_GET_ARG1(regs);
			uint8_t rval = MYVM_GET_ARG2(regs);

			logD("wr32 *r%x (@#%02x) <- r%x (#%02x)", rdest, vm->registers[rdest], rval, vm->registers[rval]);

			uint32_t dest = vm->registers[rdest];
			if (dest + 1 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx (2 bytes); max memory is #%02lx", dest, vm->memory.len);
				return false;
			}

			uint32_t val = vm->registers[rval];

			uint8_t b1 = (uint8_t)(val >> 24);
			uint8_t b2 = (uint8_t)(val >> 16);
			uint8_t b3 = (uint8_t)(val >> 8);
			uint8_t b4 = (uint8_t)val;
			vm->memory.ptr[dest] = b1;
			vm->memory.ptr[dest+1] = b2;
			vm->memory.ptr[dest+2] = b3;
			vm->memory.ptr[dest+3] = b4;
			break;
		}

		case MYVM_INS_RD8: {
			uint8_t regs = getNextByte(vm);
			uint8_t rsrc = MYVM_GET_ARG1(regs);
			uint8_t rdest = MYVM_GET_ARG2(regs);

			uint32_t src = vm->registers[rsrc];
			logD("rd8 *r%x (@#%02x) -> r%x (#%02x)", rsrc, src, rdest, vm->registers[rdest]);

			if (src >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx; max memory is #%02lx", src, vm->memory.len);
				return false;
			}

			vm->registers[rdest] = vm->memory.ptr[src];
			break;
		}

		case MYVM_INS_RD16: {
			uint8_t regs = getNextByte(vm);
			uint8_t rsrc = MYVM_GET_ARG1(regs);
			uint8_t rdest = MYVM_GET_ARG2(regs);

			uint32_t src = vm->registers[rsrc];
			logD("rd16 *r%x (@#%02x) -> r%x (#%02x)", rsrc, src, rdest, vm->registers[rdest]);

			if (src + 1 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx (2 bytes); max memory is #%02lx", src, vm->memory.len);
				return false;
			}

			uint8_t b1 = vm->memory.ptr[src];
			uint8_t b2 = vm->memory.ptr[src+1];
			vm->registers[rdest] = build16From8(b1, b2);

			break;
		}

		case MYVM_INS_RD32: {
			uint8_t regs = getNextByte(vm);
			uint8_t rsrc = MYVM_GET_ARG1(regs);
			uint8_t rdest = MYVM_GET_ARG2(regs);

			uint32_t src = vm->registers[rsrc];
			logD("rd32 *r%x (@#%02x) -> r%x (#%02x)", rsrc, src, rdest, vm->registers[rdest]);

			if (src + 3 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx (4 bytes); max memory is #%02lx", src, vm->memory.len);
				return false;
			}

			uint8_t b1 = vm->memory.ptr[src];
			uint8_t b2 = vm->memory.ptr[src+1];
			uint8_t b3 = vm->memory.ptr[src+2];
			uint8_t b4 = vm->memory.ptr[src+3];
			vm->registers[rdest] = build32From8(b1, b2, b3, b4);

			break;
		}

		default:
			logD("unknown instruction: %d", ins);
			return false;
		}
	}
}

static uint8_t getNextByte(conq_VM *vm) {
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

#include "log.h"
#include "conq.h"
#include "conq_vm.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static uint8_t getNextByte(conq_VM *vm);
static uint32_t encodeU16(uint8_t b1, uint8_t b2);
static uint32_t encodeU32(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);

#define GETREG_1(x) ((x & 0b11100000) >> 5)
#define GETREG_2(x) ((x & 0b00011100) >> 2)

bool conq_VM_init(conq_VM *dest, size_t available_memory) {
	/* int size assertions */
	assert(sizeof(uint8_t) == 1);
	assert(sizeof(uint16_t) == 2);
	assert(sizeof(uint32_t) == 4);

	memset(&dest->registers, 0, sizeof(dest->registers));
	dest->registers[CONQ_R_INSPTR] = 0x100;

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

#define CASE_2REG_INS(insname, opsym, block) \
	case CONQ_INS_ ##insname: { \
		uint8_t regs = getNextByte(vm); \
		uint8_t r1 = GETREG_1(regs); \
		uint8_t r2 = GETREG_2(regs); \
		logD("%s r%x (#%02x) %s r%x (#%02x)", #insname, r1, vm->registers[r1], opsym, r2, vm->registers[r2]); \
		block \
		break; \
	}

bool conq_VM_run(conq_VM *vm) {
	for (;;) {
		uint8_t ins = getNextByte(vm);

		switch (ins) {
		case CONQ_INS_BRK:
			logD("emulation finished");
			return true;

		case CONQ_INS_CPY: {
			uint8_t regs = getNextByte(vm);
			uint8_t rdest = GETREG_1(regs);
			uint8_t rval = GETREG_2(regs);

			logD("cpy r%x <- r%x", rdest, rval);
			vm->registers[rdest] = vm->registers[rval];
			break;
		}

		case CONQ_INS_LD8: {
			uint8_t arg1 = GETREG_1(getNextByte(vm));
			uint8_t b1 = getNextByte(vm);
			logD("ld8 r%x <- %d (#%02x)", arg1, b1, b1);
			vm->registers[arg1] = b1;
			break;
		}

		case CONQ_INS_LD16: {
			uint8_t arg1 = GETREG_1(getNextByte(vm));
			uint8_t b1 = getNextByte(vm);
			uint8_t b2 = getNextByte(vm);
			uint16_t n = encodeU16(b1, b2);
			logD("ld16 r%x <- %d (#%02x)", arg1, n, n);
			vm->registers[arg1] = encodeU16(b1, b2);
			break;
		}

		case CONQ_INS_LD32: {
			uint8_t arg1 = GETREG_1(getNextByte(vm));
			uint8_t b1 = getNextByte(vm);
			uint8_t b2 = getNextByte(vm);
			uint8_t b3 = getNextByte(vm);
			uint8_t b4 = getNextByte(vm);
			uint32_t n = encodeU32(b1, b2, b3, b4);
			logD("ld32 r%x <- %d (#%02x)", arg1, n, n);
			vm->registers[arg1] = n;
			break;
		}

		case CONQ_INS_PRINT: {
			uint8_t arg1 = GETREG_1(getNextByte(vm));
			uint32_t r = vm->registers[arg1];
			logD("r%x is %02d (#%02x)", arg1, r, r);
			break;
		}

		CASE_2REG_INS(WR8, "*<-", {
			if (r1 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx; max memory is #%02lx", r1, vm->memory.len);
				return false;
			}
			vm->memory.ptr[r1] = vm->registers[r2];
		});

		CASE_2REG_INS(WR16, "*<-", {
			if (r1 + 1 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx (2 bytes); max memory is #%02lx", r1, vm->memory.len);
				return false;
			}
			uint32_t val = vm->registers[r2];
			uint8_t b1 = (uint8_t)(val >> 8);
			uint8_t b2 = (uint8_t)val;
			vm->memory.ptr[r1] = b1;
			vm->memory.ptr[r1+1] = b2;
		});

		CASE_2REG_INS(WR32, "*<-", {
			if (r1 + 3 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx (4 bytes); max memory is #%02lx", r1, vm->memory.len);
				return false;
			}
			uint32_t val = vm->registers[r2];
			uint8_t b1 = (uint8_t)(val >> 24);
			uint8_t b2 = (uint8_t)(val >> 16);
			uint8_t b3 = (uint8_t)(val >> 8);
			uint8_t b4 = (uint8_t)val;
			vm->memory.ptr[r1] = b1;
			vm->memory.ptr[r1+1] = b2;
			vm->memory.ptr[r1+2] = b3;
			vm->memory.ptr[r1+3] = b4;
		});

		CASE_2REG_INS(RD8, "->*", {
			if (r1 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx; max memory is #%02lx", r1, vm->memory.len);
				return false;
			}
			vm->registers[r2] = vm->memory.ptr[r1];
		});

		CASE_2REG_INS(RD16, "->*", {
			if (r1 + 1 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx (2 bytes); max memory is #%02lx", r1, vm->memory.len);
				return false;
			}
			uint8_t b1 = vm->memory.ptr[r1];
			uint8_t b2 = vm->memory.ptr[r1+1];
			vm->registers[r2] = encodeU16(b1, b2);
		});

		CASE_2REG_INS(RD32, "->*", {
			if (r1 + 3 >= (uint32_t)vm->memory.len) {
				logD("address too big: got #%02lx (4 bytes); max memory is #%02lx", r1, vm->memory.len);
				return false;
			}
			uint8_t b1 = vm->memory.ptr[r1];
			uint8_t b2 = vm->memory.ptr[r1+1];
			uint8_t b3 = vm->memory.ptr[r1+2];
			uint8_t b4 = vm->memory.ptr[r1+3];
			vm->registers[r2] = encodeU32(b1, b2, b3, b4);
		});

		CASE_2REG_INS(ADD, "+=", { vm->registers[r1] += vm->registers[r2]; });
		CASE_2REG_INS(SUB, "-=", { vm->registers[r1] -= vm->registers[r2]; });
		CASE_2REG_INS(MUL, "*=", { vm->registers[r1] *= vm->registers[r2]; });
		CASE_2REG_INS(DIV, "/=", { vm->registers[r1] /= vm->registers[r2]; });
		CASE_2REG_INS(SHL, "<<=", { vm->registers[r1] <<= vm->registers[r2]; });
		CASE_2REG_INS(SHR, ">>=", { vm->registers[r1] >>= vm->registers[r2]; });

		default:
			logD("unknown instruction: %d", ins);
			return false;
		}
	}
}

static uint8_t getNextByte(conq_VM *vm) {
	size_t idx = (size_t) vm->registers[CONQ_R_INSPTR];
	if (idx >= vm->memory.len) die("instruction pointer out of bounds");
	vm->registers[CONQ_R_INSPTR]++;
	return vm->memory.ptr[idx];
}

static uint32_t encodeU32(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
	return ((uint32_t) b1 << 24) + ((uint32_t) b2 << 16) + ((uint32_t) b3 << 8) + (uint32_t) b4;
}

static uint32_t encodeU16(uint8_t b1, uint8_t b2) {
	return ((uint32_t) b1 << 8) + (uint32_t) b2;
}

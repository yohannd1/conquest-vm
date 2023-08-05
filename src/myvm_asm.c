#include "log.h"
#include "myvm_asm.h"

#include <assert.h>
#include <stdlib.h>

static bool stringEqN0(myvm_BufConst s1, const char *s2);
static bool isWhitespace(uint8_t c);

static myvm_BufConst readWord(myvm_Asm *asm);
static bool readReg(myvm_Asm *asm, uint8_t *dest);
static bool readInt(myvm_Asm *asm, uint32_t *dest);

static void myvm_Asm_write(myvm_Asm *asm, uint8_t byte);

static const myvm_BufConst ERRW_EOF = (myvm_BufConst) { .ptr = (const uint8_t *) "EOF", .len = 3 };

#define READ_REG(asm, ptr) \
	if (!readReg(asm, ptr)) return false;
#define READ_INT(asm, ptr) \
	if (!readInt(asm, ptr)) return false;

myvm_Asm myvm_Asm_init(myvm_BufConst src) {
	return (myvm_Asm) {
		.src = src,
		.src_line = 1,
		.src_i = 0,
		.rom_i = 0,
	};
}

bool myvm_Asm_compile(myvm_Asm *asm, myvm_Buf *dest_rom) {
	const size_t CHUNK_SIZE = 256;
	asm->rom = (myvm_Buf) { .ptr = NULL, .len = CHUNK_SIZE };
	asm->rom.ptr = malloc(asm->rom.len * sizeof(uint8_t));

	while (true) {
		myvm_BufConst w = readWord(asm);
		if (w.ptr == NULL) break;

		if (asm->rom_i >= asm->rom.len) {
			asm->rom.len += CHUNK_SIZE;
			asm->rom.ptr = realloc(asm->rom.ptr, asm->rom.len);
			if (asm->rom.ptr == NULL) {
				logD("OOM");
				return false;
			}
		}

		if (stringEqN0(w, "BRK")) {
			myvm_Asm_write(asm, MYVM_INS_BRK);
		} else if (stringEqN0(w, "/*")) {
			while (true) {
				w = readWord(asm);
				if (w.ptr == NULL) {
					logD("missing */");
					return false;
				}
				if (stringEqN0(w, "*/")) {
					break;
				}
			}
		} else if (stringEqN0(w, "LD8")) {
			uint8_t reg;
			READ_REG(asm, &reg);

			uint32_t n;
			READ_INT(asm, &n);

			myvm_Asm_write(asm, MYVM_INS_LD8);
			myvm_Asm_write(asm, (reg & 0b111) << 5);
			myvm_Asm_write(asm, (uint8_t) n);
		} else if (stringEqN0(w, "LD16")) {
			uint8_t reg;
			READ_REG(asm, &reg);

			uint32_t n;
			READ_INT(asm, &n);

			myvm_Asm_write(asm, MYVM_INS_LD16);
			myvm_Asm_write(asm, (reg & 0b111) << 5);
			myvm_Asm_write(asm, (uint8_t) (n >> 8));
			myvm_Asm_write(asm, (uint8_t) n);
		} else if (stringEqN0(w, "LD32")) {
			uint8_t reg;
			READ_REG(asm, &reg);

			uint32_t n;
			READ_INT(asm, &n);

			myvm_Asm_write(asm, MYVM_INS_LD32);
			myvm_Asm_write(asm, (reg & 0b111) << 5);
			myvm_Asm_write(asm, (uint8_t) (n >> 24));
			myvm_Asm_write(asm, (uint8_t) (n >> 16));
			myvm_Asm_write(asm, (uint8_t) (n >> 8));
			myvm_Asm_write(asm, (uint8_t) n);
		} else if (stringEqN0(w, "PRINT")) {
			uint8_t reg;
			READ_REG(asm, &reg);

			myvm_Asm_write(asm, MYVM_INS_PRINT);
			myvm_Asm_write(asm, (reg & 0b111) << 5);
		} else {
			logD("line %d: unknown word: %.*s", asm->src_line, w.len, w.ptr);
			return false;
		}
	}

	/* try to tighten memory but it's not critical, as `len` is tight already. */
	uint8_t *tighter = realloc(asm->rom.ptr, asm->rom.len);
	if (tighter != NULL) asm->rom.ptr = tighter;

	*dest_rom = asm->rom;
	return true;
}

static myvm_BufConst readWord(myvm_Asm *asm) {
	/* skip whitespace and count lines */
	for (; asm->src_i < asm->src.len; asm->src_i += 1) {
		char c = asm->src.ptr[asm->src_i];
		if (c == '\n') asm->src_line += 1;
		if (isWhitespace(c)) continue;
		break;
	}

	/* count word chars */
	size_t start = asm->src_i;
	for (; !isWhitespace(asm->src.ptr[asm->src_i]) && asm->src_i < asm->src.len;
	     asm->src_i += 1)
		;

	return (start == asm->src_i)
		   ? (myvm_BufConst) { .ptr = NULL, .len = 0 }
		   : (myvm_BufConst) { .ptr = &asm->src.ptr[start], .len = asm->src_i - start };
}

static bool stringEqN0(myvm_BufConst s1, const char *s2) {
	size_t i = 0;
	while (true) {
		if (i >= s1.len && s2[i] == '\0') return true; /* both ended */
		if (i >= s1.len || s2[i] == '\0') return false; /* only one ended */
		if (s1.ptr[i] != s2[i]) return false; /* different chars */
		i++;
	}
}

static bool isWhitespace(uint8_t c) {
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static bool readReg(myvm_Asm *asm, uint8_t *dest) {
	myvm_BufConst w = readWord(asm);
	if (w.ptr == NULL) {
		w = ERRW_EOF;
		goto error;
	}

	if (w.len != 2 || w.ptr[0] != 'r' || w.ptr[1] < '0' || w.ptr[1] > '7') goto error;

	*dest = w.ptr[1] - '0';
	assert((*dest & 0b111) == *dest);
	return true;

error:
	logD("expected register (r0-r7), found %.*s", w.len, w.ptr);
	return false;
}

static bool readInt(myvm_Asm *asm, uint32_t *dest) {
	myvm_BufConst w = readWord(asm);
	if (w.ptr == NULL) {
		w = ERRW_EOF;
		goto error;
	}

	uint32_t mul = 1;
	uint32_t result = 0;
	size_t j = 0;
	for (; j < w.len; j++) {
		char c = w.ptr[w.len - 1 - j];
		if (c < '0' || c > '9') goto error;
		result += (uint32_t) (c - '0') * mul;
		mul *= 10;
	}

	*dest = result;
	return true;

error:
	logD("expected integer, got %.*s", w.len, w.ptr);
	return false;
}

static void myvm_Asm_write(myvm_Asm *asm, uint8_t byte) {
	asm->rom.ptr[asm->rom_i] = byte;
	asm->rom_i++;
}

#include "log.h"
#include "myvm_asm.h"

#include <stdlib.h>
#include <assert.h>

static bool stringEqN0(myvm_BufConst s1, const char *s2);
static bool isWhitespace(uint8_t c);

static myvm_BufConst readWord(myvm_BufConst src, size_t *i, size_t *line);
static bool readReg(uint8_t *dest, myvm_BufConst src, size_t *i, size_t *line);
static bool readInt(uint32_t *dest, myvm_BufConst src, size_t *i, size_t *line);

static const myvm_BufConst ERRW_EOF = (myvm_BufConst) { .ptr = (const uint8_t*)"EOF", .len = 3 };

#define READ_REG(ptr) \
	if (!readReg(ptr, src_code, &src_i, &line)) return false;
#define READ_INT(ptr) \
	if (!readInt(ptr, src_code, &src_i, &line)) return false;

bool myvm_asm_compile(myvm_Buf *dest_rom, myvm_BufConst src_code) {
	size_t line = 1;
	size_t src_i = 0;
	size_t rom_i = 0;

	const size_t CHUNK_SIZE = 256;
	myvm_Buf buf = (myvm_Buf) { .ptr = NULL, .len = CHUNK_SIZE };
	buf.ptr = malloc(buf.len * sizeof(uint8_t));

	while (true) {
		myvm_BufConst w = readWord(src_code, &src_i, &line);
		if (w.ptr == NULL) break;

		if (rom_i >= buf.len) {
			buf.len += CHUNK_SIZE;
			buf.ptr = realloc(buf.ptr, buf.len);
			if (buf.ptr == NULL) {
				logD("OOM");
				return false;
			}
		}

		if (stringEqN0(w, "BRK")) {
			buf.ptr[rom_i] = MYVM_INS_BRK;
			rom_i++;
		} else if (stringEqN0(w, "/*")) {
			while (true) {
				w = readWord(src_code, &src_i, &line);
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
			READ_REG(&reg);

			uint32_t n;
			READ_INT(&n);

			buf.ptr[rom_i] = MYVM_INS_LD8;
			rom_i++;

			buf.ptr[rom_i] = (reg & 0b111) << 5;
			rom_i++;

			buf.ptr[rom_i] = (uint8_t) n;
			rom_i++;
		} else if (stringEqN0(w, "LD16")) {
			uint8_t reg;
			READ_REG(&reg);

			uint32_t n;
			READ_INT(&n);

			buf.ptr[rom_i] = MYVM_INS_LD16;
			rom_i++;

			buf.ptr[rom_i] = (reg & 0b111) << 5;
			rom_i++;

			buf.ptr[rom_i] = (uint8_t) (n >> 8);
			rom_i++;

			buf.ptr[rom_i] = (uint8_t) n;
			rom_i++;
		} else if (stringEqN0(w, "LD32")) {
			uint8_t reg;
			READ_REG(&reg);

			uint32_t n;
			READ_INT(&n);

			buf.ptr[rom_i] = MYVM_INS_LD32;
			rom_i++;

			buf.ptr[rom_i] = (reg & 0b111) << 5;
			rom_i++;

			buf.ptr[rom_i] = (uint8_t) (n >> 24);
			rom_i++;

			buf.ptr[rom_i] = (uint8_t) (n >> 16);
			rom_i++;

			buf.ptr[rom_i] = (uint8_t) (n >> 8);
			rom_i++;

			buf.ptr[rom_i] = (uint8_t) n;
			rom_i++;
		} else if (stringEqN0(w, "PRINT")) {
			uint8_t reg;
			READ_REG(&reg);

			buf.ptr[rom_i] = MYVM_INS_PRINT;
			rom_i++;

			buf.ptr[rom_i] = (reg & 0b111) << 5;
			rom_i++;
		} else {
			logD("line %d: unknown word: %.*s", line, w.len, w.ptr);
			return false;
		}
	}

	/* try to tighten memory but it's not critical, as `len` is tight already. */
	uint8_t *tighter = realloc(buf.ptr, buf.len);
	if (tighter != NULL) buf.ptr = tighter;

	*dest_rom = buf;
	return true;
}

static myvm_BufConst readWord(myvm_BufConst src, size_t *i, size_t *line) {
	/* skip whitespace and count lines */
	for (; *i < src.len; *i += 1) {
		char c = src.ptr[*i];
		if (c == '\n') *line += 1;
		if (isWhitespace(c)) continue;
		break;
	}

	/* count word chars */
	size_t start = *i;
	for (; !isWhitespace(src.ptr[*i]) && *i < src.len; *i += 1)
		;

	return (start == *i) ? (myvm_BufConst) { .ptr = NULL, .len = 0 }
			     : (myvm_BufConst) { .ptr = &src.ptr[start], .len = *i - start };
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

static bool readReg(uint8_t *dest, myvm_BufConst src, size_t *i, size_t *line) {
	myvm_BufConst w = readWord(src, i, line);
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

static bool readInt(uint32_t *dest, myvm_BufConst src, size_t *i, size_t *line) {
	myvm_BufConst w = readWord(src, i, line);
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

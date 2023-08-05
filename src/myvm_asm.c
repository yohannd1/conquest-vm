#include "myvm_asm.h"
#include "log.h"

#include <stdlib.h>

static myvm_BufConst readWord(myvm_BufConst src, size_t *i, size_t *line);
static bool stringEqN0(myvm_BufConst s1, const char *s2);
static bool isWhitespace(uint8_t c);

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
			buf.ptr[rom_i] = MYVM_INS_BREAK;
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
	for (; !isWhitespace(src.ptr[*i]) && *i < src.len; *i += 1);

	return (start == *i)
		? (myvm_BufConst) { .ptr = NULL, .len = 0 }
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

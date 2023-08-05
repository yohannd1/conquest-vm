#ifndef _MYVM_H
#define _MYVM_H

#include <stddef.h>
#include <stdint.h>

#define MYVM_INS_BREAK	 0 /* stops execution */
#define MYVM_INS_COPY	 1 /* copies the value from arg2 to arg1 */
#define MYVM_INS_LOAD	 2 /* loads a constant value (arg2) into arg1 */
#define MYVM_INS_LOADIF	 3 /* same as load, but only does it if the bool flag is true */
#define MYVM_INS_READ8	 4 /* reads a byte from memory address arg2 into arg1 */
#define MYVM_INS_READ16	 5 /* reads a word from memory address arg2 into arg1 */
#define MYVM_INS_READ32	 6 /* reads a 32-bit word from memory address arg2 into arg1 */
#define MYVM_INS_WRITE8	 7
#define MYVM_INS_WRITE16 8
#define MYVM_INS_WRITE32 9

/* these instructions change the leftmost bit of the flags register */
#define MYVM_CMP_EQ
#define MYVM_CMP_L
#define MYVM_CMP_LE
#define MYVM_CMP_G
#define MYVM_CMP_GE
#define MYVM_CMP_NOT
#define MYVM_NOT

#define MYVM_R_INSPTR 6
#define MYVM_R_FLAGS  7

typedef struct {
	uint8_t *ptr;
	size_t len;
} myvm_Buf;

typedef struct {
	const uint8_t *ptr;
	size_t len;
} myvm_BufConst;

#define myvm_BufConst_from(b) \
	(myvm_BufConst) { .ptr = b.ptr, .len = b.len }

#endif

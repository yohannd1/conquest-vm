#ifndef _MYVM_H
#define _MYVM_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
	MYVM_INS_BRK = 0, /* stop VM */
	MYVM_INS_CPY,
	MYVM_INS_JMPIF, /* increase PC by ARG1 if bool flag is true */

	/* load */
	MYVM_INS_LD8,
	MYVM_INS_LD16,
	MYVM_INS_LD32,

	/* read (TODO: rename to fetch?) */
	MYVM_INS_RD8,
	MYVM_INS_RD16,
	MYVM_INS_RD32,

	/* write (TODO: rename to store?) */
	MYVM_INS_WR8,
	MYVM_INS_WR16,
	MYVM_INS_WR32,

	/* debug instruction */
	MYVM_INS_PRINT,
} conq_Instruction;

/* these instructions change the leftmost bit of the flags register */
/* TODO */
/* #define MYVM_CMP_EQ */
/* #define MYVM_CMP_L */
/* #define MYVM_CMP_LE */
/* #define MYVM_CMP_G */
/* #define MYVM_CMP_GE */
/* #define MYVM_CMP_NOT */
/* #define MYVM_NOT */

#define MYVM_R_INSPTR 6
#define MYVM_R_FLAGS  7

typedef struct {
	uint8_t *ptr;
	size_t len;
} conq_Buf;

typedef struct {
	const uint8_t *ptr;
	size_t len;
} conq_BufConst;

#define conq_BufConst_from(b) \
	(conq_BufConst) { .ptr = b.ptr, .len = b.len }

#endif

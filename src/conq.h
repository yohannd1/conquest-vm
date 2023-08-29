#ifndef _CONQ_H
#define _CONQ_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
	CONQ_INS_BRK = 0, /* stop VM */
	CONQ_INS_CPY, /* copy the value of one register to another */
	CONQ_INS_JMPIF, /* increase PC by ARG1 if bool flag is true */

	/* load */
	CONQ_INS_LD8,
	CONQ_INS_LD16,
	CONQ_INS_LD32,

	/* read (TODO: rename to fetch?) */
	CONQ_INS_RD8,
	CONQ_INS_RD16,
	CONQ_INS_RD32,

	/* write (TODO: rename to store?) */
	CONQ_INS_WR8,
	CONQ_INS_WR16,
	CONQ_INS_WR32,

	/* arithmetic */
	CONQ_INS_ADD,
	CONQ_INS_SUB,
	CONQ_INS_MUL,
	CONQ_INS_DIV,
	CONQ_INS_SHL,
	CONQ_INS_SHR,

	/* debug instruction */
	CONQ_INS_PRINT,

	/* TODO: CONQ_INS_ASSERT */
} conq_Instruction;

/* these instructions change the leftmost bit of the flags register */
/* TODO */
/* #define CONQ_CMP_EQ */
/* #define CONQ_CMP_L */
/* #define CONQ_CMP_LE */
/* #define CONQ_CMP_G */
/* #define CONQ_CMP_GE */
/* #define CONQ_CMP_NOT */
/* #define CONQ_NOT */

#define CONQ_R_INSPTR 6
#define CONQ_R_FLAGS  7

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

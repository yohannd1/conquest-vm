#ifndef _CONQ_ASM_H
#define _CONQ_ASM_H

#include "conq.h"

#include <stdbool.h>

typedef struct conq_Asm {
	conq_BufConst src;
	conq_Buf rom;
	size_t src_line;
	size_t src_i;
	size_t rom_i;
} conq_Asm;

conq_Asm conq_Asm_init(conq_BufConst src);
bool conq_Asm_compile(conq_Asm *asm, conq_Buf *dest_rom);

#endif

#ifndef _MYVM_ASM_H
#define _MYVM_ASM_H

#include "myvm.h"

#include <stdbool.h>

typedef struct myvm_Asm {
	myvm_BufConst src;
	myvm_Buf rom;
	size_t src_line;
	size_t src_i;
	size_t rom_i;
} myvm_Asm;

myvm_Asm myvm_Asm_init(myvm_BufConst src);
bool myvm_Asm_compile(myvm_Asm *asm, myvm_Buf *dest_rom);

#endif

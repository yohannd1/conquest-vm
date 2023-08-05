#ifndef _MYVM_ASM_H
#define _MYVM_ASM_H

#include "myvm.h"

#include <stdbool.h>

bool myvm_asm_compile(myvm_Buf *dest_rom, myvm_BufConst src_code);

#endif

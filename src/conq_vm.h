#ifndef _CONQ_VM_H
#define _CONQ_VM_H

#include "conq.h"

#include <stdbool.h>

typedef struct conq_VM {
	uint32_t registers[8];
	conq_Buf memory;
	/* TODO: how to do device mapping (first device: terminal) */
} conq_VM;

bool conq_VM_init(conq_VM *dest, size_t available_memory);
void conq_VM_deinit(conq_VM *vm);
bool conq_VM_copyRom(conq_VM *vm, conq_BufConst rom);
bool conq_VM_run(conq_VM *vm);

#endif /* _CONQ_VM_H */

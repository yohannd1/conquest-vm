#ifndef _MYVM_VM_H
#define _MYVM_VM_H

#include "myvm.h"

#include <stdbool.h>

typedef struct myvm_VM {
	uint32_t registers[8];
	myvm_Buf memory;
	/* TODO: how to do device mapping (first device: terminal) */
} myvm_VM;

bool myvm_VM_init(myvm_VM *dest, size_t available_memory);
void myvm_VM_deinit(myvm_VM *vm);
bool myvm_VM_copyRom(myvm_VM *vm, myvm_BufConst rom);
bool myvm_VM_run(myvm_VM *vm);

#endif /* _MYVM_VM_H */

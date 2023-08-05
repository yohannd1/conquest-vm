#include "log.h"
#include "myvm_asm.h"
#include "myvm_vm.h"

#include <stddef.h>

int main(int argc, char *argv[]) {
	if (argc != 2) die("need exactly one argument (asm file name)");

	logD("opening file %s...", argv[1]);
	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) die("failed to open file %s", argv[1]);

	const size_t CHUNK_SIZE = 512;
	myvm_Buf fbuf = (myvm_Buf) { .ptr = NULL, .len = 0 };
	fbuf.len = CHUNK_SIZE;
	fbuf.ptr = malloc(CHUNK_SIZE);
	if (fbuf.ptr == NULL) die("OOM");
	size_t i = 0;
	for (;; i++) {
		int c = fgetc(fp);
		if (c == EOF) {
			fbuf.len = i;
			/* try to tighten memory but it's not critical. */
			uint8_t *tighter = realloc(fbuf.ptr, fbuf.len);
			if (tighter != NULL) fbuf.ptr = tighter;
			break;
		}
		if (i >= fbuf.len) {
			fbuf.len += CHUNK_SIZE;
			fbuf.ptr = realloc(fbuf.ptr, fbuf.len);
			if (fbuf.ptr == NULL) die("OOM");
		}
		fbuf.ptr[i] = (uint8_t) c;
	}
	fclose(fp);

	myvm_Buf rom;
	if (!myvm_asm_compile(&rom, myvm_BufConst_from(fbuf))) {
		die("failed to compile to ROM");
	}

	myvm_VM vm;
	if (!myvm_VM_init(&vm, 64000)) die("failed to init VM");
	if (!myvm_VM_copyRom(&vm, myvm_BufConst_from(rom))) {
		die("failed to copy ROM to VM memory");
	}
	/* for (i = 0x100; i < 0x200; i++) { */
	/* 	logD("rom[#%02x] = #%02x", i, vm.memory.ptr[i]); */
	/* } */
	if (!myvm_VM_run(&vm)) die("VM exited with failure");

	return 0;
}

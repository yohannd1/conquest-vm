#include "log.h"
#include "conq_asm.h"
#include "conq_vm.h"

#include <stddef.h>

int main(int argc, char *argv[]) {
	if (argc != 2) die("need exactly one argument (asm file name)");

	logD("opening file %s...", argv[1]);
	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) die("failed to open file %s", argv[1]);

	const size_t CHUNK_SIZE = 512;
	conq_Buf fbuf = (conq_Buf) { .ptr = NULL, .len = 0 };
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

	conq_Buf rom;
	conq_Asm asm = conq_Asm_init(conq_BufConst_from(fbuf));
	if (!conq_Asm_compile(&asm, &rom)) {
		die("failed to compile to ROM");
	}

	conq_VM vm;
	if (!conq_VM_init(&vm, 64000)) die("failed to init VM");
	if (!conq_VM_copyRom(&vm, conq_BufConst_from(rom))) {
		die("failed to copy ROM to VM memory");
	}
	/* for (i = 0x100; i < 0x200; i++) { */
	/* 	logD("rom[#%02x] = #%02x", i, vm.memory.ptr[i]); */
	/* } */
	if (!conq_VM_run(&vm)) die("VM exited with failure");

	return 0;
}

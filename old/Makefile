BUILD_DIR := build
PROGRAM := $(BUILD_DIR)/conq_test
SRC := $(wildcard src/*.c)
CFLAGS = -std=c89 -Wall -O3

$(PROGRAM): $(SRC) $(BUILD_DIR)
	gcc -o $@ $(SRC) $(CFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(PROGRAM)
	$(PROGRAM) test/src.txt

.PHONY: run

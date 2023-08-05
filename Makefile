BUILD_DIR := build
PROGRAM := $(BUILD_DIR)/myvm_test
SRC := $(wildcard src/*.c)
CFLAGS = -std=c89 -Wall

run: $(PROGRAM)
	$(PROGRAM) test/src.txt

$(PROGRAM): $(SRC) $(BUILD_DIR)
	gcc -o $@ $(SRC) $(CFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

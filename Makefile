CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude
LDFLAGS :=

TARGET  := simple-shell
BUILD   := build
SRC     := $(wildcard src/*.c)
OBJ     := $(patsubst src/%.c,$(BUILD)/%.o,$(SRC))

.PHONY: all clean test asan asan-test debug

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

test_parser: $(BUILD)/test_parser.o $(BUILD)/parser.o
	$(CC) $(CFLAGS) $^ -o $(BUILD)/test_parser $(LDFLAGS)

$(BUILD)/test_parser.o: tests/test_parser.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET) test_parser
	./$(BUILD)/test_parser
	bash tests/test_shell.sh ./$(TARGET)

asan: clean
	$(MAKE) CFLAGS='$(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer' LDFLAGS='-fsanitize=address,undefined' all

asan-test: clean
	$(MAKE) CFLAGS='$(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer' LDFLAGS='-fsanitize=address,undefined' test

clean:
	rm -rf $(BUILD) $(TARGET) $(TARGET).exe

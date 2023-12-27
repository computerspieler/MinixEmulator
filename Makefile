SRCS=$(wildcard src/*.c) $(wildcard libx86emu/*.c)
DEPS=$(patsubst %,bin/deps/%.d,$(SRCS))
OBJS=$(patsubst %,bin/objs/%.o,$(SRCS))

CC=gcc
LD=gcc
CCFLAGS=-Wall -Wextra -g -ggdb -Isrc -Iinclude -Ilibx86emu/include -c \
	-Wno-incompatible-pointer-types
LDFLAGS=-g -ggdb

all: bin/emulator

clean:
	rm -rf bin

-include $(DEPS)

bin/emulator: $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

bin/objs/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -o $@ $<

bin/deps/%.c.d: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -M -o $@ $< -MT $(patsubst bin/deps/%,bin/objs/%,$@)


SRCS=$(wildcard src/*.c) $(wildcard libx86emu/*.c)
DEPS=$(patsubst %,bin/deps/%.d,$(SRCS))
OBJS=$(patsubst %,bin/objs/%.o,$(SRCS))

CC=gcc
LD=gcc
CCFLAGS=-Wall -Wextra -Iinclude -Ilibx86emu/include -c \
	-Wno-incompatible-pointer-types -Wno-unused-parameter
LDFLAGS=

all: debug

release: LDFLAGS:=-O2
release: CCFLAGS:=$(CCFLAGS) -O2
release: bin/emulator

debug: LDFLAGS:=-g -ggdb -fsanitize=address
debug: CCFLAGS:=$(CCFLAGS) -g -ggdb -DDEBUG
debug: bin/emulator

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


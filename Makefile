SRCS=$(wildcard src/*.c) $(wildcard libx86emu/*.c)
DEPS=$(patsubst %,bin/deps/%.d,$(SRCS))
DEBUGOBJS=$(patsubst %,bin/debug/objs/%.o,$(SRCS))
RELEASEOBJS=$(patsubst %,bin/release/objs/%.o,$(SRCS))

CC=gcc
LD=gcc
CCFLAGS=-Wall -Wextra -I$(PWD)/include -I$(PWD)/libx86emu/include -c \
	-Wno-incompatible-pointer-types -Wno-unused-parameter
LDFLAGS=

all: debug

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif

release: bin/release/emulator
debug: bin/debug/emulator

clean:
	rm -rf bin

bin/debug/emulator: $(DEBUGOBJS)
	$(LD) -o $@ $^ $(LDFLAGS) -g -ggdb -fsanitize=address

bin/debug/objs/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -g -ggdb -DDEBUG -o $@ $<

bin/release/emulator: $(RELEASEOBJS)
	$(LD) -o $@ $^ $(LDFLAGS) -O2

bin/release/objs/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -O2 -o $@ $<

bin/deps/%.c.d: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -M -o $@ $< -MT $(patsubst bin/deps/%.d,bin/objs/%.o,$@)


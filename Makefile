SRC=$(wildcard src/*.c)

INCLUDES?=
INCLUDES+=-I src

override CFLAGS?=-Wall -std=c99

include lib/.dep/config.mk

override CFLAGS+=$(INCLUDES)
override CFLAGS+=-D_DEFAULT_SOURCE
override CFLAGS+=$(shell pkg-config --cflags openssl)
override LDFLAGS+=$(shell pkg-config --libs openssl)

OBJ=$(SRC:.c=.o)

BIN=\
	benchmark \
	test

default: README.md $(BIN)
# default: README.md $(BIN) libhttpc.a libhttpc.so

# libhttpc.a: $(OBJ)
# 	ar rcs $@ $^

# libhttpc.so: $(OBJ)
# 	$(CC) $(OBJ) --shared -o $@

$(BIN): $(OBJ) $(BIN:=.o)
	$(CC) $(LDFLAGS) $(OBJ) $@.o -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)
	rm -f $(BIN:=.o)
	rm -f test

README.md: ${SRC} src/httpc.h
	stddoc < src/httpc.h > README.md


SRC=$(wildcard src/*.c) inih/ini.c
OBJ=$(sort $(SRC:.c=.o))
BIN=mmre
HEADER=include/mmre.h
CC=clang
CFLAGS= \
	-Iinclude \
	-I/usr/include/libxml2 \
	-Iinih \
	-Wall \
	-g \
	-Wno-unused-label \
	-Wno-pointer-to-int-cast \
	-fsanitize=address
LD= -lxml2 -lcurl

TEST_SRC=$(wildcard tests/*.c)
TESTS=$(sort $(TEST_SRC:.c=.test))

.PHONY: all clean tests
all: $(BIN)

clean:
	@rm -rf $(OBJ) $(BIN) $(LIB) $(TESTS)

%.o: %.c $(HEADER)
	@echo CC $@
	@$(CC) $(CFLAGS) -o $@ -c $<

$(BIN): $(OBJ)
	@echo LD $@
	@$(CC) $(CFLAGS) $(LD) -o $@ $(OBJ)

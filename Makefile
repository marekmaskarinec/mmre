
SRC=$(wildcard src/*.c) inih/ini.c
OBJ=$(sort $(SRC:.c=.o))
BIN=mmre
HEADER=include/mmre.h
CFLAGS= \
	-Iinclude \
	-I/usr/include/libxml2 \
	-Iinih \
	-Wall \
	-DNDEBUG \
	-Wno-unused-label \
	-Wno-pointer-to-int-cast \
	-Wno-unused-result \
	-O2
LD= -lxml2 -lcurl

TEST_SRC=$(wildcard tests/*.c)
TESTS=$(sort $(TEST_SRC:.c=.test))

.PHONY: all clean install
all: $(BIN)

clean:
	@rm -rf $(OBJ) $(BIN) $(LIB) $(TESTS)

%.o: %.c $(HEADER)
	@echo CC $@
	@$(CC) $(CFLAGS) -o $@ -c $<

$(BIN): $(OBJ)
	@echo LD $@
	@$(CC) $(CFLAGS) -o $@ $(OBJ) $(LD)

install: $(BIN) mmre.1 mmre.5
	install mmre /usr/bin
	cp mmre.1 /usr/share/man/man1
	cp mmre.5 /usr/share/man/man5
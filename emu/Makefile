CC = gcc
CRELFLAGS = -std=c99
CDEVFLAGS = -pedantic -Werror -Wall -Wextra -Wvla -Wshadow -g
LDLIBS = -lm -lraylib -lGL

SRC = `find . ../utils -name '*.c'`
OBJ = `find . ../utils -name '*.o'`
SSA = `find . -name '*.ssa'`
ASM = `find . -name '*.asm'`
EXE = mymsed

all: setrel comp

comp:
	$(CC) -o $(EXE) $(CFLAGS) $(LDLIBS) $(SRC)
rel: setrel comp
dev: setdev comp
run:
	./$(EXE)
gdb: dev
	gdb ./$(EXE)
memcheck: dev
	valgrind --leak-check=full -s ./$(EXE) ../sample.ml
setdev:
	$(eval CFLAGS := $(CDEVFLAGS))
setrel:
	$(eval CFLAGS := $(CRELFLAGS))

.PHONY: setdev setrel debug mop clean

mop:
	$(RM) $(OBJ)
	$(RM) $(SSA)
	$(RM) $(ASM)

clean: mop
	$(RM) $(EXE)

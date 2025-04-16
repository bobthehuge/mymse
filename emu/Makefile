CC = clang
CRELFLAGS = -std=c99 -O0
CDEVFLAGS = -std=c99 -g
LDLIBS =

SRC = `find . ../utils -name '*.c'`
OBJ = `find . ../utils -name '*.o'`
SSA = `find . -name '*.ssa'`
ASM = `find . -name '*.asm'`
EXE = mymsed

all: setrel comp

comp:
	$(CC) -o $(EXE) $(SRC) $(CFLAGS) $(LDLIBS)
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

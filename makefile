CC	= gcc
CFLAGS	= -g -Wall -Wextra -pedantic -std=c99 -O3
LIBS	= -lm

ASM_SRC	= Assembler/*.c
ASM_OUT	= asm

VM_SRC	= VM/*.c
VM_OUT	= run

default: a v

a: $(ASM_SRC)
	$(CC) $(CFLAGS) $(ASM_SRC) -o $(ASM_OUT)

v: $(VM_SRC)
	$(CC) $(CFLAGS) $(VM_SRC) -o $(VM_OUT) $(LIBS)

clean:
	rm -f $(ASM_OUT) $(VM_OUT)
	rm -Rf $(ASM_OUT).dSYM $(VM_OUT).dSYM
	rm -f fbf7c968
	rm -f 62fe98d2

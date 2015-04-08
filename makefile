CC	= gcc
CFLAGS	= -g -Wall -Wextra -pedantic -std=c99
LIBS	= -lm

ASM_SRC	= RiscyAssembler/*.c
ASM_OUT	= asm

VM_SRC	= RiscyVM/*.c
VM_OUT	= vm

default: asm vm

asm: $(ASM_SRC)
	$(CC) $(CFLAGS) $(ASM_SRC) -o $(ASM_OUT)

vm: $(VM_SRC)
	$(CC) $(CFLAGS) $(VM_SRC) -o $(VM_OUT) $(LIBS)

clean:
	rm -f $(ASM_OUT) $(VM_OUT)
	rm -Rf $(ASM_OUT).dSYM $(VM_OUT).dSYM

#include "vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct RiscyVM {
	uint16_t	regs	[NUM_REGISTERS];
	uint16_t	program	[MEMORY_SIZE];
	uint16_t	pc;

	uint16_t	data_size;	/* Number of lines of data */
	uint16_t	data_start;	/* Start address of data */
	uint16_t	text_header;	/* The address of the text header */
	uint16_t	text_size;	/* Number of lines of text */
	uint16_t	text_start;	/* Start address of text */

	bool		is_running;
};

/* TODO(Alexander):
 * 	- Do _NOT_ forget that addi, sw, lw and beq all take _signed_ immediates
 * 	  as their last argument. Handle this ASAP. 
 */
static void add	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC);
static void addi(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void nand(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC);
static void lui	(RiscyVM* vm, uint16_t regA, uint16_t uimm);
static void sw	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void lw	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void beq	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void jalr(RiscyVM* vm, uint16_t regA, uint16_t regB);

static uint16_t load_to_array_from_file(uint16_t array[], FILE* file);
static uint16_t create_mask(int a, int b);

RiscyVM* VM_init(char filename[])
{
	/* Attempt to open the binary program file */
	FILE* file;
	if ((file = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "[!] Invalid file: \"%s\".\n", filename);
		exit(EXIT_FAILURE);
	}

	RiscyVM* vm = malloc(sizeof(RiscyVM));

	/* Initialize all registers to hold the value 0 */
	memset(vm->regs, 0, sizeof vm->regs);

	/* Load each line of the binary program into the VM's program array */
	int num_lines = load_to_array_from_file(vm->program, file);

	/* Close the file; we don't need it anymore */
	rewind(file);
	fclose(file);

	/* _size is the value in the address of the header.
	 * _start is the address of the first line of the data/text.
	 */
	vm->data_size	= vm->program[0];
	vm->data_start	= 1;

	vm->text_size	= vm->program[vm->data_start + vm->data_size];
	vm->text_header	= vm->data_start + vm->data_size + 1;
	vm->text_start	= vm->text_header + 1;

	printf("vm->data_size  = %d\n", vm->data_size);
	printf("vm->data_start = %d\n", vm->data_start);
	printf("vm->text_size  = %d\n", vm->text_size);
	printf("vm->text_start = %d\n", vm->text_start);

	/* Set program counter to point to the first instruction */
	vm->pc = vm->text_start;
	printf("vm->pc         = %d\n", vm->pc);

	/* Set the running flag */
	vm->is_running = true;

	return vm;
}

void VM_shutdown(RiscyVM* vm)
{
	if (vm != NULL)
		free(vm);
}

bool VM_is_running(RiscyVM* vm)
{
	return vm->is_running;
}

uint16_t fetch(RiscyVM* vm)
{
	printf("PC = %d\n", vm->pc);
	if (vm->pc == vm->text_header + vm->text_size - 1) {
		printf("Executing last instruction. <-------------- [!]\n");
		vm->is_running = false;
	}

	return vm->program[vm->pc++];
}

/* Instruction masks */
#define ADD	(0x000)
#define ADDI	(0x001)
#define NAND	(0x002)
#define LUI	(0x003)
#define SW	(0x004)
#define LW	(0x005)
#define BEQ	(0x006)
#define JALR	(0x007)

#define REG_A	(

void decode(RiscyVM* vm, uint16_t instruction)
{
	uint16_t
	opcode_mask, regA_mask, regB_mask, regC_mask, simm_mask, uimm_mask;

	uint16_t
	opcode, regA, regB, regC, simm, uimm;

	opcode_mask	= create_mask(13, 16);
	regA_mask	= create_mask(10, 13);
	regB_mask	= create_mask(7 , 10);
	regC_mask	= create_mask(0 , 3 );
	simm_mask	= create_mask(0 , 7 );
	uimm_mask	= create_mask(0 , 10);

	opcode	= (instruction & opcode_mask) >> (16 - 3);
	regA	= (instruction & regA_mask) >> (16 - 6);
	regB	= (instruction & regB_mask) >> (16 - 9);
	regC	= (instruction & regC_mask);
	simm	= (instruction & simm_mask);
	uimm	= (instruction & simm_mask);

	/* TODO(Alexander):
	 * 	- WTF am I doing? The execute function should execute the
	 * 	  instructions! Change this!
	 */
	switch (opcode) {
	case ADD:
		printf("opcode = ADD\n");
		add(vm, regA, regB, regC);
		break;

	case ADDI:
		printf("opcode = ADDI\n");
		break;

	case NAND:
		printf("opcode = NAND\n");
		break;

	case LUI:
		printf("opcode = LUI\n");
		break;

	case SW:
		printf("opcode = SW\n");
		break;

	case LW:
		printf("opcode = LW\n");
		break;

	case BEQ:
		printf("opcode = BEQ\n");
		break;

	case JALR:
		printf("opcode = JALR\n");
		break;
	}

	return;
}

void execute() { }

static void add(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC) {}
static void addi(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm) {}
static void nand(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC) {}
static void lui(RiscyVM* vm, uint16_t regA, uint16_t uimm) {}
static void sw(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm) {}
static void lw(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm) {}
static void beq(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm) {}
static void jalr(RiscyVM* vm, uint16_t regA, uint16_t regB) {}

static uint16_t load_to_array_from_file(uint16_t array[], FILE* file)
{
	uint16_t	num_lines = 0;
	char		buffer[WORD_SIZE + 1 + 1];

	while (fgets(buffer, sizeof buffer, file)) {
		strtok(buffer, "\n");	/* Strip the newline character */
		array[num_lines++] = (uint16_t) strtol(buffer, NULL, 2);
	}

	printf("Done loading program. Printing loaded values:\n");
	printf("-------------\n");
	for (int i = 0; i < num_lines; ++i) {
		printf("\t%x\n", array[i]);
	}
	printf("-------------\n");

	return num_lines;
}

static uint16_t create_mask(int a, int b)
{
	if (a > b) {
		fprintf(stderr, "[!] %s: %s: Not allowed (a > b).\n",
				__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	uint16_t result = 0;

	for (int i = a; i <= b; ++i) {
		result |= 1 << i;
	}
	return result;
}


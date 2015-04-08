#include "vm.h"
#include "macros.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants  */
#define MEMORY_SIZE		(0xffff)	/* 2^16 - 1 */
#define STACK_BOTTOM		(MEMORY_SIZE)
#define WORD_SIZE		(16)		/* bits */
#define NUM_REGISTERS		(8)

/* Instructions */
#define ADD	(0x000)
#define ADDI	(0x001)
#define NAND	(0x002)
#define LUI	(0x003)
#define SW	(0x004)
#define LW	(0x005)
#define BEQ	(0x006)
#define JALR	(0x007)

/* Instruction masks */
#define MASK_OPCODE	(0xe000)	/* 1110 0000 0000 0000 */
#define MASK_REG_A	(0x1c00)	/* 0001 1100 0000 0000 */
#define MASK_REG_B	(0x0380)	/* 0000 0011 1000 0000 */
#define MASK_REG_C	(0x0007)	/* 0000 0000 0000 0111 */
#define MASK_SIMM	(0x007f)	/* 0000 0000 0111 1111 */
#define MASK_UIMM	(0x03ff)	/* 0000 0011 1111 1111 */

/* If true, more information will be printed. Defined in vm_main.c. */
extern bool print_verbose_output;

/* Utility functions */
static uint16_t	load_to_array_from_file	(uint16_t array[], FILE* file);
static char*	dec_to_bin		(char* bin, int dec, int nbr_bits);
static void	sign_n_bits		(uint16_t* s, unsigned int n);

typedef struct	metadata_t	metadata_t;
typedef struct	instruction_t	instruction_t;

struct metadata_t {
	uint16_t	data_size;	/* Number of lines of data */
	uint16_t	data_start;     /* Start address of data */
	uint16_t	text_header;    /* The address of the text header */
	uint16_t	text_size;      /* Number of lines of text */
	uint16_t	text_start;     /* Start address of text */
};

struct instruction_t {
	uint16_t	opcode;		/* Operation code */
	uint16_t	regA;		/* Register A */
	uint16_t	regB;		/* Register B */
	uint16_t	regC;		/* Register C */
	uint16_t	simm;		/* Signed immediate */
	uint16_t	uimm;		/* Unsigned immediate */
};

struct RiscyVM {
	uint16_t	regs[NUM_REGISTERS];	/* Registers */
	uint16_t	program[MEMORY_SIZE];	/* Integer instructions */
	uint16_t	pc;			/* Program counter */

	metadata_t	metadata;		/* Information about program */
	instruction_t	current_instruction;	/* Instruction executed during
						   the current cycle */

	bool		is_running;		/* PC != last instruction */
};

RiscyVM* VM_init(char filename[])
{
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		ERROR("\t%s", OUT_OF_MEMORY);
	}

	RiscyVM* vm = malloc(sizeof *vm);
	if (vm == NULL) {
		ERROR("\t%s", OUT_OF_MEMORY);
	}

	/* Initialize all registers to hold the value 0 */
	memset(vm->regs, 0, sizeof vm->regs);

	/* Set r7 (stack pointer) to point to the top of the stack. */
	vm->regs[7] = STACK_BOTTOM;
	DEBUG_VAR("", vm->regs[7], "\t(Stack Pointer)\n", "0x%04x");

	/* Load each line of the binary program into the VM's program array */
	if (print_verbose_output)
		printf("Loading values from file \"%s\" ... ", filename);
	int num_lines = load_to_array_from_file(vm->program, file);
	if (print_verbose_output)
		printf("%d lines loaded from \"%s\".\n\n", num_lines, filename);

	/* Close the file; we don't need it anymore */
	rewind(file);
	fclose(file);

	/* _size is the value in the address of the header.
	 * _start is the address of the first line of the data/text.
	 */
	metadata_t* md	= &vm->metadata;
	md->data_size	= vm->program[0];
	md->data_start	= 1;
	md->text_size	= vm->program[md->data_start + md->data_size];
	md->text_header = md->data_start + md->data_size;
	md->text_start	= md->text_header + 1;

	/* Debug metadata */
	DEBUG_VAR("", md->data_start	, "\n", PRINT_FORMAT);
	DEBUG_VAR("", md->data_size	, "\n", PRINT_FORMAT);
	DEBUG_VAR("", md->text_header	, "\n", PRINT_FORMAT);
	DEBUG_VAR("", md->data_start	, "\n", PRINT_FORMAT);
	DEBUG_VAR("", md->data_start	, "\n", PRINT_FORMAT);

	/* Set program counter to point to the first instruction */
	vm->pc = md->text_start;
	DEBUG_VAR("", vm->pc, "\n\n", PRINT_FORMAT);

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

void VM_print_regs(RiscyVM* vm)
{
	uint16_t* r = vm->regs;

	printf
	(
	"+------------+------------+------------+------------+\n"
	"| " KRED "r0" RESET ": " KGRN TABLE_PRINT_FORMAT RESET " "
	"| " KRED "r1" RESET ": " KGRN TABLE_PRINT_FORMAT RESET " "
	"| " KRED "r2" RESET ": " KGRN TABLE_PRINT_FORMAT RESET " "
	"| " KRED "r3" RESET ": " KGRN TABLE_PRINT_FORMAT RESET " |\n"
	"+------------+------------+------------+------------+\n"
	"| " KRED "r4" RESET ": " KGRN TABLE_PRINT_FORMAT RESET " "
	"| " KRED "r5" RESET ": " KGRN TABLE_PRINT_FORMAT RESET " "
	"| " KRED "r6" RESET ": " KGRN TABLE_PRINT_FORMAT RESET " "
	"| " KRED "r7" RESET ": " KGRN TABLE_PRINT_FORMAT RESET " |\n"
	"+------------+------------+------------+------------+\n"
	"| " KRED "pc" RESET ": " KGRN TABLE_PRINT_FORMAT RESET " |\n"
	"+------------+\n",
	r[0], r[1], r[2], r[3],
	r[4], r[5], r[6], r[7],
	vm->pc
	);
}

void VM_print_data(RiscyVM* vm)
{
	for (int i = 0; i < vm->metadata.data_size; ++i) {
		printf("Data[ %2d ] = "PRINT_FORMAT"\n", i,
			vm->program[vm->metadata.data_start + i]);
	}
}

void VM_fetch(RiscyVM* vm)
{
	if (vm->pc >= vm->metadata.text_header + vm->metadata.text_size)
		vm->is_running = false;

	vm->pc += 1;
}

void VM_decode(RiscyVM* vm)
{
	uint16_t instruction	= vm->program[vm->pc - 1];

	uint16_t opcode		= (instruction & MASK_OPCODE) >> (16 - 3);
	uint16_t regA		= (instruction & MASK_REG_A) >> (16 - 6);
	uint16_t regB		= (instruction & MASK_REG_B) >> (16 - 9);
	uint16_t regC		= (instruction & MASK_REG_C);
	uint16_t simm		= (instruction & MASK_SIMM);
	uint16_t uimm		= (instruction & MASK_UIMM);

	if (print_verbose_output) {
		char binbuf[17];
		printf("%s\n", dec_to_bin(binbuf, instruction, 16));
	}

	/* If the MSB of simm is 1, convert to the negative version */
	sign_n_bits(&simm, 7);

	/* The contents of register 0 should always be 0 */
	if (regA == 0)	{ vm->regs[regA] = 0; }
	if (regB == 0)	{ vm->regs[regB] = 0; }
	if (regC == 0)	{ vm->regs[regC] = 0; }

	DEBUG_VAR("", opcode,	"\n", PRINT_FORMAT);
	DEBUG_VAR("", regA,	"\n", PRINT_FORMAT);
	DEBUG_VAR("", regB,	"\n", PRINT_FORMAT);
	DEBUG_VAR("", regC,	"\n", PRINT_FORMAT);
	DEBUG_VAR("", simm,	"\n", PRINT_FORMAT);
	DEBUG_VAR("", uimm,	"\n", PRINT_FORMAT);

	vm->current_instruction = (instruction_t) { opcode, regA, regB, regC,
							simm, uimm };
}

void VM_execute(RiscyVM* vm)
{
	uint16_t	opcode	= vm->current_instruction.opcode;
	uint16_t	regA	= vm->current_instruction.regA;
	uint16_t	regB	= vm->current_instruction.regB;
	uint16_t	regC	= vm->current_instruction.regC;
	uint16_t	simm	= vm->current_instruction.simm;
	uint16_t	uimm	= vm->current_instruction.uimm;

	switch (opcode) {
	case ADD:
		vm->regs[regA] = vm->regs[regB] + vm->regs[regC];
		if (print_verbose_output)
			printf("add r%d, r%d, r%d\n", regA, regB, regC);
		break;

	case ADDI:
		vm->regs[regA] = vm->regs[regB] + simm;
		if (print_verbose_output)
			printf("addi r%d, r%d, "PRINT_FORMAT"\n",
				regA, regB, simm);
		break;

	case NAND:
		vm->regs[regA] = ~(vm->regs[regB] & vm->regs[regC]);
		if (print_verbose_output)
			printf("nand r%d, r%d, r%d\n", regA, regB, regC);
		break;

	case LUI:
		vm->regs[regA] = uimm << 6;

		if (print_verbose_output)
			printf("lui r%d, "PRINT_FORMAT"\n", regA, uimm);

		if ((vm->regs[regA] & 0x3f) != 0)
			printf("%s: %s: LUI: Something went wrong!\n",
					__FILE__, __func__);
		break;

	case SW:
		vm->program[vm->regs[regB] + simm] = vm->regs[regA];
		if (print_verbose_output)
			printf("sw r%d, r%d, "PRINT_FORMAT"\n",
				regA, regB, simm);
		break;

	case LW:
		if (print_verbose_output)
			printf("lw r%d, r%d, "PRINT_FORMAT"\n", regA, regB, simm);
		vm->regs[regA] = vm->program[vm->regs[regB] + simm];
		break;

	case BEQ:
		if (vm->regs[regA] == vm->regs[regB]) {
			vm->pc += simm;
			if (print_verbose_output)
				printf("<< Equal contents >>\n");
		}
		if (print_verbose_output)
			printf("beq r%d, r%d, "PRINT_FORMAT"\n", regA, regB, simm);
		break;

	case JALR:
		vm->regs[regA] = vm->pc;
		vm->pc = vm->regs[regB];

		if (print_verbose_output)
			printf("jalr r%d, r%d\n", regA, regB);
		break;
	}
}

static uint16_t load_to_array_from_file(uint16_t array[], FILE* file)
{
	uint16_t	num_lines = 0;
	char		buffer[WORD_SIZE + 1 + 1];

	while (fgets(buffer, sizeof buffer, file)) {
		strtok(buffer, "\n");
		array[num_lines++] = (uint16_t) strtol(buffer, NULL, 16);
	}

	if (print_verbose_output) {
		printf("done.\nPrinting loaded addresses and values:\n");
		printf("-------------\n");
		printf("    Address    Value\n");
		for (int i = 0; i < num_lines; ++i) {
			printf("    %6d:    0x%04x", i, array[i]);

			printf("%s\n",	i == 0		  ? "  <-- Data header":
					i == array[0] + 1 ? "  <-- Text header":
					"");
		}
		printf("-------------\n");
	}

	return num_lines;
}

static char* dec_to_bin(char* bin, int dec, int nbr_bits)
{
	int i;
	bin[nbr_bits] = '\0';
	for (i = nbr_bits - 1; i >= 0; --i, dec >>= 1) {
		bin[i] = (dec & 1) + '0';
	}
	return bin;
}

static void sign_n_bits(uint16_t* s, unsigned int n)
{
	if (*s > pow(2, n - 1) - 1) {
		DEBUG_PRINT("simm: 0x%04x -> ", *s);
		*s -= pow(2, n);
		DEBUG_PRINT("0x%04x\n", *s);
	}
}


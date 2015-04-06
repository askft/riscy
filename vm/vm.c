#include "vm.h"
#include "print_format.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE		(0xffff)	/* 2^16 - 1 */
#define WORD_SIZE		(16)
#define NUM_REGISTERS		(8)

// TODO(Alexander): Would this be better to use than the block below?
//enum { ADD, ADDI, NAND, LUI, SW, LW, BEQ, JALR, }
	
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
	instruction_t	current_instruction;

	bool		is_running;		/* PC != last instruction */
};


//static void	test_mask		(uint16_t mask, uint16_t result);
static void	sign_n_bits		(uint16_t* s, unsigned int n);
static uint16_t	load_to_array_from_file	(uint16_t array[], FILE* file);

RiscyVM* VM_init(char filename[])
{
	/* Attempt to open the binary program file */
	FILE* file;
	if ((file = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "[!] Invalid file: \"%s\".\n", filename);
		exit(EXIT_FAILURE);
	}

	RiscyVM* vm = malloc(sizeof(RiscyVM));
	if (vm == NULL) {
		fprintf(stderr, "[!] %s: %s: Out of memory.\n",
				__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	/* Initialize all registers to hold the value 0 */
	memset(vm->regs, 0, sizeof vm->regs);

	/* Set r7 to point to the top of the stack */
	vm->regs[7] = MEMORY_SIZE;
	printf("vm->regs[7] aka Stack Pointer = 0x%04x\n", vm->regs[7]);

	/* Load each line of the binary program into the VM's program array */
	int num_lines = load_to_array_from_file(vm->program, file);
	printf("%d lines loaded from %s.\n", num_lines, filename);

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

	/* Print metadata */
	printf("vm->metadata.data_start  = "PRINT_FORMAT"\n", md->data_start);
	printf("vm->metadata.data_size   = "PRINT_FORMAT"\n", md->data_size);
	printf("vm->metadata.text_header = "PRINT_FORMAT"\n", md->text_header);
	printf("vm->metadata.text_start  = "PRINT_FORMAT"\n", md->text_start);
	printf("vm->metadata.text_size   = "PRINT_FORMAT"\n", md->text_size);

	/* Set program counter to point to the first instruction */
	vm->pc = md->text_start;
	printf("vm->pc = "PRINT_FORMAT"\n", vm->pc);

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
	if (vm->pc == vm->metadata.text_header + vm->metadata.text_size) {
		printf("Executing last instruction. <-------------- [!]\n");
		vm->is_running = false;
	}

	vm->pc += 1;
}

char* dec_to_bin(char* bin, int dec, int nbr_bits)
{
	int i;
	bin[nbr_bits] = '\0';
	for (i = nbr_bits - 1; i >= 0; --i, dec >>= 1) {
		bin[i] = (dec & 1) + '0';
	}
	return bin;
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

	char binbuf[17];
	printf("%s\n", dec_to_bin(binbuf, instruction, 16));

	/* If the MSB of simm is 1, convert to the negative version */
	sign_n_bits(&simm, 7);

	/* The contents of register 0 should always be 0 */
	if (regA == 0)	{ vm->regs[regA] = 0; }
	if (regB == 0)	{ vm->regs[regB] = 0; }
	if (regC == 0)	{ vm->regs[regC] = 0; }

//	printf("opcode = %d\n", opcode); printf("regA = %d\n", regA);
//	printf("regB = %d\n", regB); printf("regC = %d\n", regC);
//	printf("simm = %d\n", simm); printf("uimm = %d\n", uimm);

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
		printf("add r%d, r%d, r%d\n", regA, regB, regC);
		vm->regs[regA] = vm->regs[regB] + vm->regs[regC];
		break;

	case ADDI:
		printf("addi r%d, r%d, "PRINT_FORMAT"\n", regA, regB, simm);
		vm->regs[regA] = vm->regs[regB] + simm;
		break;

	case NAND:
		printf("nand r%d, r%d, r%d\n", regA, regB, regC);
		vm->regs[regA] = ~(vm->regs[regB] & vm->regs[regC]);
		break;

	case LUI:
		printf("lui r%d, "PRINT_FORMAT"\n", regA, uimm);
	
		/* [uimm] is a 16 bit number with the 6 MSB's AND:ed to 0.
		 * Shifting it left by 6 will set it to nnnnnnnnnn000000. */
		vm->regs[regA] = uimm << 6;

		if ((vm->regs[regA] & 0x3f) != 0) {
			printf("%s: %s: LUI: Something went wrong!\n",
					__FILE__, __func__);
		}
		break;

	case SW:
		printf("sw r%d, r%d, "PRINT_FORMAT"\n", regA, regB, simm);
		vm->program[vm->regs[regB] + simm] = vm->regs[regA];
		break;

	case LW:
		printf("lw r%d, r%d, "PRINT_FORMAT"\n", regA, regB, simm);
		vm->regs[regA] = vm->program[vm->regs[regB] + simm];
		break;

	case BEQ:
		printf("beq r%d, r%d, "PRINT_FORMAT"\n", regA, regB, simm);
		if (vm->regs[regA] == vm->regs[regB]) {
			printf("<< Equal contents >>\n");
			vm->pc = simm;
		}
		break;

	case JALR:
		printf("jalr r%d, r%d\n", regA, regB);
		/* regA shall be reserved on calls as it contains the address of the
		 * calling instruction */
		vm->regs[regA] = vm->pc; /* XXX Changed from vm->pc + 1 */
		vm->pc = vm->regs[regB];
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

	printf("Done loading program. Printing loaded values:\n");
	printf("-------------\n");
	for (int i = 0; i < num_lines; ++i) {
		printf("\t0x%04x\n", array[i]);
	}
	printf("-------------\n");

	return num_lines;
}

static void sign_n_bits(uint16_t* s, unsigned int n)
{
	if (*s > pow(2, n - 1) - 1) {
		printf("simm: 0x%04x -> ", *s);
		*s -= pow(2, n);
		printf("0x%04x\n", *s);
	}
}

#if 0
static void test_mask(uint16_t mask, uint16_t result)
{
	if (mask != result)
		printf("[!] Invalid mask 0x%x.\n", mask);
}
#endif

//==============================================================================

//==============================================================================

//==============================================================================

//==============================================================================

//==============================================================================

#if 0
static void add	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC);
static void addi(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void nand(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC);
static void lui	(RiscyVM* vm, uint16_t regA, uint16_t uimm);
static void sw	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void lw	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void beq	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void jalr(RiscyVM* vm, uint16_t regA, uint16_t regB);
#endif

#if 0
static void add(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC)
{
	vm->regs[regA] = vm->regs[regB] + vm->regs[regC];
}

static void addi(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm)
{
	vm->regs[regA] = vm->regs[regB] + simm;
}

static void nand(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC)
{
	vm->regs[regA] = ~(vm->regs[regB] & vm->regs[regC]);
}

static void lui(RiscyVM* vm, uint16_t regA, uint16_t uimm) 
{
//	vm->regs[regA] = uimm & 0xffc0;	/* Load upper 10 bits of [uimm] */
//
//	// XXX New
//
//	vm->regs[regA] = uimm & 0xffc0;
//	vm->regs[regA] = vm->regs[regA] << 6;

	// XXX This should be correct. [uimm] is a 16 bit number with the
	// 6 most significant bits AND:ed to 0. Shifting it left by 6 will
	// set it to nnnnnnnnnn000000
	vm->regs[regA] = uimm << 6;

	if ((vm->regs[regA] & 0x3f) != 0) {
		printf("LUI: Something went wrong!\n");
	}
}

static void sw(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm)
{
	vm->program[vm->regs[regB] + simm] = vm->regs[regA];
}

static void lw(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm)
{
	vm->regs[regA] = vm->program[vm->regs[regB] + simm];
}

static void beq(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm)
{
	if (regA == regB) {
		printf("<< Unconditional branch >>\n");
	}
	if (vm->regs[regA] == vm->regs[regB]) {
		printf("<< Equal contents >>\n");
		vm->pc = simm;
	}
}

static void jalr(RiscyVM* vm, uint16_t regA, uint16_t regB)
{
	/* regA shall be reserved on calls as it contains the address of the
	 * calling instruction */
	vm->regs[regA] = vm->pc + 1;
	vm->pc = vm->regs[regB];
}
#endif


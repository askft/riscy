#include "vm.h"
#include "term_color.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO(Alexander)
 * 	Should every integer have the type uint16_t? The only time I would want
 * 	them as int16_t is when printing them. Right?
 */

#define MEMORY_SIZE		(0x7fff)	/* 2^15 - 1 */
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

// XXX(Format)
/* Instruction format */
//typedef enum { RRR, RRI, RI, } iformat_t;

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
	// XXX(Format)
//	iformat_t	format;		/* Instruction format (RRR/RRI/RI) */
	uint16_t	opcode;		/* Operation code */
	uint16_t	regA;		/* Register A */
	uint16_t	regB;		/* Register B */
	uint16_t	regC;		/* Register C */
	uint16_t	simm;		/* Signed immediate */
	uint16_t	uimm;		/* Unsigned immediate */
};

struct RiscyVM {
	int16_t		regs[NUM_REGISTERS];	/* Registers */
	uint16_t	program[MEMORY_SIZE];	/* Integer instructions */
//	int16_t*	data;			/* Signed 16 bit data array */
	uint16_t	pc;			/* Program counter */

	metadata_t	metadata;		/* Information about program */
	instruction_t	current_instruction;

	bool		is_running;		/* PC != last instruction */
};


static void	test_mask		(uint16_t mask, uint16_t result);
static void	sign_n_bits		(int16_t* s, unsigned int n);
static uint16_t	load_to_array_from_file	(uint16_t array[], FILE* file);

static void add	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC);
static void addi(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm);
static void nand(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC);
static void lui	(RiscyVM* vm, uint16_t regA, uint16_t uimm);
static void sw	(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm);
static void lw	(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm);
static void beq	(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm);
static void jalr(RiscyVM* vm, uint16_t regA, uint16_t regB);

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

	/* Set r7 to point to the top of the stack */
	vm->regs[7] = MEMORY_SIZE;
	printf("vm->regs[7] = %"PRIu16"\n", vm->regs[7]);

	/* Load each line of the binary program into the VM's program array */
	int num_lines = load_to_array_from_file(vm->program, file);

	/* Close the file; we don't need it anymore */
	rewind(file);
	fclose(file);

	metadata_t* md = &vm->metadata;

	/* _size is the value in the address of the header.
	 * _start is the address of the first line of the data/text.
	 */
	md->data_size	= vm->program[0];
	md->data_start	= 1;

	md->text_size	= vm->program[md->data_start + md->data_size];
	md->text_header = md->data_start + md->data_size;
	md->text_start	= md->text_header + 1;

	/* Print metadata */
	printf("vm->metadata.data_start  = %d\n", md->data_start);
	printf("vm->metadata.data_size   = %d\n", md->data_size);
	printf("vm->metadata.text_header = %d\n", md->text_header);
	printf("vm->metadata.text_start  = %d\n", md->text_start);
	printf("vm->metadata.text_size   = %d\n", md->text_size);

	/* Set program counter to point to the first instruction */
	vm->pc = md->text_start;
	printf("vm->pc = %d\n", vm->pc);

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
	int16_t* r = vm->regs;
	int width = 5;

	printf
	(
	"+-----------+-----------+-----------+-----------+\n"
	"| " KRED "r0" RESET ": " KGRN "%*d" RESET " "
	"| " KRED "r1" RESET ": " KGRN "%*d" RESET " "
	"| " KRED "r2" RESET ": " KGRN "%*d" RESET " "
	"| " KRED "r3" RESET ": " KGRN "%*d" RESET " |\n"
	"+-----------+-----------+-----------+-----------+\n"
	"| " KRED "r4" RESET ": " KGRN "%*d" RESET " "
	"| " KRED "r5" RESET ": " KGRN "%*d" RESET " "
	"| " KRED "r6" RESET ": " KGRN "%*d" RESET " "
	"| " KRED "r7" RESET ": " KGRN "%*d" RESET " |\n"
	"+-----------+-----------+-----------+-----------+\n"
	"| " KRED "pc" RESET ": " KGRN "%*d" RESET " "             "\n"
	"+-----------+-----------+-----------+-----------+\n",
	width, r[0], width, r[1], width, r[2], width, r[3],
	width, r[4], width, r[5], width, r[6], width, r[7],
	width, vm->pc
	);
}

void VM_print_data(RiscyVM* vm)
{
	for (int i = 0; i < vm->metadata.data_size; ++i) {
		printf("Data[ %*d ] = %"PRId16"\n", 2, i,
			(int16_t) vm->program[vm->metadata.data_start + i]);
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
	uint16_t	instruction = vm->program[vm->pc - 1];

//	char binbuf[17];
//	printf("%s\n", dec_to_bin(binbuf, instruction, 16));

	// XXX(Format)
//	iformat_t	format;
	uint16_t	opcode;
	uint16_t	regA;
	uint16_t	regB;
	uint16_t	regC;
	int16_t		simm;
	uint16_t	uimm;

	test_mask(MASK_OPCODE	, 57344	); test_mask(MASK_REG_A	, 7168	);
	test_mask(MASK_REG_B	, 896	); test_mask(MASK_REG_C	, 7	);
	test_mask(MASK_SIMM	, 127	); test_mask(MASK_UIMM	, 1023	);

	opcode	= (instruction & MASK_OPCODE) >> (16 - 3);
	regA	= (instruction & MASK_REG_A) >> (16 - 6);
	regB	= (instruction & MASK_REG_B) >> (16 - 9);
	regC	= (instruction & MASK_REG_C);
	simm	= (instruction & MASK_SIMM);
	uimm	= (instruction & MASK_UIMM);

	/* If the MSB of simm is 1, convert to the negative version */
	sign_n_bits(&simm, 7);

	/* The contents of register 0 should always be 0 */
	if (regA == 0)	{ vm->regs[regA] = 0; }
	if (regB == 0)	{ vm->regs[regB] = 0; }
	if (regC == 0)	{ vm->regs[regC] = 0; }

//	printf("opcode = %d\n", opcode);
//	printf("regA = %d\n", regA);
//	printf("regB = %d\n", regB);
//	printf("regC = %d\n", regC);
//	printf("simm = %d\n", simm);
//	printf("uimm = %d\n", uimm);

	// XXX(Format)
//	switch (opcode) {
//	case ADD:
//	case NAND:
//		format = RRR;
//		break;
//	case ADDI:
//	case SW:
//	case LW:
//	case BEQ:
//	case JALR:
//		format = RRI;
//		break;
//	case LUI:
//		format = RI;
//		break;
//	}

	// XXX(Format)
	vm->current_instruction = (instruction_t) {/*	format,*/
							opcode,
							regA, regB, regC,
							simm, uimm };
}

/* TODO(Alexander)
 * 	- The functions for each of the instructions need not exits;
 * 	  their contents could simply be called in each case in the
 * 	  switch below. That would reduce the line count by about 30-35.
 */
void VM_execute(RiscyVM* vm)
{
	uint16_t	opcode	= vm->current_instruction.opcode;
	uint16_t	regA	= vm->current_instruction.regA;
	uint16_t	regB	= vm->current_instruction.regB;
	uint16_t	regC	= vm->current_instruction.regC;
	int16_t		simm	= vm->current_instruction.simm;
	uint16_t	uimm	= vm->current_instruction.uimm;

	switch (opcode) {
	case ADD:
		printf("add r%d, r%d, r%d\n", regA, regB, regC);
		add(vm, regA, regB, regC);
		break;

	case ADDI:
		printf("addi r%d, r%d, %"PRId16"\n", regA, regB, simm);
		addi(vm, regA, regB, simm);
		break;

	case NAND:
		printf("nand r%d, r%d, r%d\n", regA, regB, regC);
		nand(vm, regA, regB, regC);
		break;

	case LUI:
		printf("lui r%d, %d\n", regA, uimm);
		lui(vm, regA, uimm);
		break;

	case SW:
		printf("sw r%d, r%d, %"PRId16"\n", regA, regB, simm);
		sw(vm, regA, regB, simm);
		break;

	case LW:
		printf("lw r%d, r%d, %"PRId16"\n", regA, regB, simm);
		lw(vm, regA, regB, simm);
		break;

	case BEQ:
		printf("beq r%d, r%d, %"PRId16"\n", regA, regB, simm);
		beq(vm, regA, regB, simm);
		break;

	case JALR:
		printf("jalr r%d, r%d\n", regA, regB);
		jalr(vm, regA, regB);
		break;
	}
}

static void add(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC)
{
	vm->regs[regA] = vm->regs[regB] + vm->regs[regC];
}

static void addi(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm)
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

static void sw(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm)
{
	vm->program[vm->regs[regB] + simm] = vm->regs[regA];
}

static void lw(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm)
{
	vm->regs[regA] = vm->program[vm->regs[regB] + simm];
}

static void beq(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm)
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

static uint16_t load_to_array_from_file(uint16_t array[], FILE* file)
{
	uint16_t	num_lines = 0;
	char		buffer[WORD_SIZE + 1 + 1];

	while (fgets(buffer, sizeof buffer, file)) {
		strtok(buffer, "\n");
		array[num_lines++] = (uint16_t) strtol(buffer, NULL, 16);
	//	array[num_lines++] = (uint16_t) strtol(buffer, NULL, 10);
	}

	printf("Done loading program. Printing loaded values:\n");
	printf("-------------\n");
	for (int i = 0; i < num_lines; ++i) {
		printf("\t0x%04x\n", array[i]);
	//	printf("\t%"PRIu16"\n", array[i]);
	}
	printf("-------------\n");

	return num_lines;
}

static void test_mask(uint16_t mask, uint16_t result)
{
	if (mask != result)
		printf("[!] Invalid mask 0x%x.\n", mask);
}

static void sign_n_bits(int16_t* s, unsigned int n)
{
	if (*s > pow(2, n - 1) - 1) {
		printf("simm: %"PRId16" -> ", *s);
		*s -= pow(2, n);
		printf("%"PRId16".\n", *s);
	}
}


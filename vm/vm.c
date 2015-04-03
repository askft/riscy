#include "vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO(Alexander): Would these be better to use than the two blocks below?
//enum { ADD, ADDI, NAND, LUI, SW, LW, BEQ, JALR, }
//enum { REG_0, REG_1, REG_2 REG_3, REG_4, REG_5, REG_6, REG_7, }
	
/* Instructions */
#define ADD	(0x000)
#define ADDI	(0x001)
#define NAND	(0x002)
#define LUI	(0x003)
#define SW	(0x004)
#define LW	(0x005)
#define BEQ	(0x006)
#define JALR	(0x007)

/* Registers */
#define REG_0	(0x000)
#define REG_1	(0x001)
#define REG_2	(0x002)
#define REG_3	(0x003)
#define REG_4	(0x004)
#define REG_5	(0x005)
#define REG_6	(0x006)
#define REG_7	(0x007)

/* Instruction masks */
#define MASK_OPCODE	(0xe000)	/* 1110 0000 0000 0000 */
#define MASK_REG_A	(0x1c00)	/* 0001 1100 0000 0000 */
#define MASK_REG_B	(0x0380)	/* 0000 0011 1000 0000 */
#define MASK_REG_C	(0x0007)	/* 0000 0000 0000 0111 */
#define MASK_SIMM	(0x007f)	/* 0000 0000 0111 1111 */
#define MASK_UIMM	(0x03ff)	/* 0000 0011 1111 1111 */

/* Instruction format */
typedef enum { RRR, RRI, RI, } iformat_t;

typedef struct {
	uint16_t	data_size;	/* Number of lines of data */
	uint16_t	data_start;     /* Start address of data */
	uint16_t	text_header;    /* The address of the text header */
	uint16_t	text_size;      /* Number of lines of text */
	uint16_t	text_start;     /* Start address of text */
} metadata_t;

struct instruction_t {
	iformat_t	format;		/* Instruction format (RRR/RRI/RI) */
	uint16_t	opcode;		/* Operation code */
	uint16_t	regA;		/* Register A */
	uint16_t	regB;		/* Register B */
	uint16_t	regC;		/* Register C */
	uint16_t	simm;		/* Signed immediate */
	uint16_t	uimm;		/* Unsigned immediate */
};

struct RiscyVM {
	uint16_t	regs[NUM_REGISTERS];	/* Registers */
	uint16_t	program[MEMORY_SIZE];	/* Integer instruction */
	uint16_t	pc;			/* Program counter */

	metadata_t	metadata;		/* Information about program  */
	instruction_t	current_instruction;

	bool		is_running;
};

static void	test_mask		(uint16_t mask, uint16_t result);
static void	sign7bits		(int16_t* s);
static uint16_t	load_to_array_from_file	(uint16_t array[], FILE* file);
//static uint16_t create_mask		(int a, int b);


/* TODO(Alexander):
 * 	- Do _NOT_ forget that addi, sw, lw and beq all take _signed_ immediates
 * 	  as their last argument. Handle this ASAP. 
 */
static void add	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC);
static void addi(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm);
static void nand(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC);
static void lui	(RiscyVM* vm, uint16_t regA, uint16_t uimm);
static void sw	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void lw	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
static void beq	(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t simm);
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

	printf("vm->data_size  = %d\n", md->data_size);
	printf("vm->data_start = %d\n", md->data_start);
	printf("vm->text_size  = %d\n", md->text_size);
	printf("vm->text_start = %d\n", md->text_start);

	/* Set program counter to point to the first instruction */
	/* TODO(Alexander): Make sure the PC is correct. */
//	vm->pc = md->text_header;
	vm->pc = md->text_start;
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

void VM_fetch(RiscyVM* vm)
{
	printf("PC = %d\n", vm->pc);
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

	char binbuf[17];
	printf("Instruction: __%s__\n", dec_to_bin(binbuf, instruction, 16));

	iformat_t	format;
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

	printf("opcode = %d\n", opcode);
	printf("regA = %d\n", regA);
	printf("regB = %d\n", regB);
	printf("regC = %d\n", regC);
	printf("simm = %d\n", simm);
	printf("uimm = %d\n", uimm);

	switch (opcode) {
	case ADD:
	case NAND:
		format = RRR;
		break;
	case ADDI:
	case SW:
	case LW:
	case BEQ:
	case JALR:
		format = RRI;
		break;
	case LUI:
		format = RI;
		break;
	}

	vm->current_instruction = (instruction_t) {	format,
							opcode,
							regA, regB, regC,
							simm, uimm };
}

void VM_execute(RiscyVM* vm)
{
	uint16_t	opcode	= vm->current_instruction.opcode;
	uint16_t	regA	= vm->current_instruction.regA;
	uint16_t	regB	= vm->current_instruction.regB;
	uint16_t	regC	= vm->current_instruction.regC;
	int32_t		simm	= vm->current_instruction.simm;
	uint16_t	uimm	= vm->current_instruction.uimm;

	/* TODO(Alexander):
	 * 	- Everything that needs to be done in this switch =)
	 */

	switch (opcode) {
	case ADD:
		printf("opcode = ADD\n");
		add(vm, regA, regB, regC);
		break;

	case ADDI:
		printf("opcode = ADDI\n");
		addi(vm, regA, regB, simm);
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
}

static void add(RiscyVM* vm, uint16_t regA, uint16_t regB, uint16_t regC)
{
	vm->regs[regA] = vm->regs[regB] + vm->regs[regC];
}

static void addi(RiscyVM* vm, uint16_t regA, uint16_t regB, int16_t simm)
{
	vm->regs[regA] = vm->regs[regB] + simm;
}

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
//		printf("Last 7 = ");
//		for (int i = 9; i <= 16; ++i)
//			printf("%c", buffer[i]);
//		printf("\n");
	}

	printf("Done loading program. Printing loaded values:\n");
	printf("-------------\n");
	for (int i = 0; i < num_lines; ++i) {
		printf("\t%x\n", array[i]);
	}
	printf("-------------\n");

	return num_lines;
}

static void test_mask(uint16_t mask, uint16_t result)
{
	if (mask != result)
		printf("[!] Invalid mask 0x%x.\n", mask);
}

static void sign7bits(int16_t* s)
{
	printf("Converted %d to ", *s);
	if (*s > 63)
		*s -= 128;
	printf("%d.\n", *s);
}

//static uint16_t create_mask(int a, int b)
//{
//	if (a > b) {
//		fprintf(stderr, "[!] %s: %s: Not allowed (a > b).\n",
//				__FILE__, __func__);
//		exit(EXIT_FAILURE);
//	}
//
//	uint16_t result = 0;
//
//	for (int i = a; i <= b; ++i) {
//		result |= 1 << i;
//	}
//	return result;
//}


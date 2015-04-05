#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "riscy.h"
#include "utility.h"

#define MAX_LINE_LENGTH	(1024)
#define MAX		(1024)
#define WORD_SIZE	(16)

#define MASK_LOW_7	(0x7f)		/* 0000 0000 0111 1111 */
#define MASK_LOW_10	(0x3ff)		/* 0000 0011 1111 1111 */
#define MASK_UPP_9	(0xff80)	/* 1111 1111 1000 0000 */

/* Splits up a line into proper assembly language tokens */
static int	tokenize	(char**		tokens,
				 const char*	src,
				 const char*	delimiters);

/* Assembles [src], a line with instructions, and stores the result in [dest] */
static void	assemble_line	(uint16_t*	dest,
				 uint16_t*	line_num,
				 const char*	src);


/* File Cleanup 
 * 	Remove comments and trailing whitespace.
 * 	Store the result in a new file [output].
 */
void file_cleanup(FILE* output, FILE* input)
{
	char buffer[MAX_LINE_LENGTH];

	while (fgets(buffer, sizeof buffer, input)) {
		remove_comments(buffer);
		remove_trailing_whitespace(buffer);
		if (strlen(buffer) > MAX_LINE_LENGTH) {
			printf("[!] Line too long: %s\n", buffer); 
			exit(EXIT_FAILURE);
		}
		fprintf(output, "%s", buffer);
	}
}

/* XXX currently unused */
void file_remove_blank_lines(FILE* output, FILE* input)
{
	char buffer[160 + 1];

	while (fgets(buffer, sizeof buffer, input)) {
		if (!is_empty_line(buffer)) {
			fprintf(output, "%s", buffer);
		}
	}
}

/* Parse Labels
 * 		If a label such as "example:" is found, store the address
 * 		which is 0 to 2^16 - 1 in the label list.
 */
void parse_labels(FILE* input, label_list_t* list)
{
	char		buffer[MAX_LINE_LENGTH];
	char*		token;
	uint16_t	address = 0;

	while (fgets(buffer, sizeof buffer, input)) {

		if (is_empty_line(buffer))
			continue;

		token = strtok(buffer, "\t\n ,");

		if (strlast(token) == ':') {

			if (buffer[0] != token[0]) {
				printf("[!] %u: Labels may not be indented.\n",
					address);
				exit(EXIT_FAILURE);
			}

			token[strlen(token) - 1] = '\0';
			label_list_add(list, token, address);
		}

		address += 1;
	}
	label_list_print(list);
	rewind(input);		/* A real programmer doesn't litter. */
}

/* Reduces the input by removing all labels "to the left", i.e. those that end
 * with a ':', and also replaces the lables given as arguments to instructions
 * with their binary representations, i.e. their real addresses.
 */
void replace_labels(FILE* output, FILE* input, label_list_t* list)
{
	char		buffer_in[MAX_LINE_LENGTH];
	char		buffer_out[MAX_LINE_LENGTH];
	char		buffer_str[MAX_LINE_LENGTH];
	char*		token;
	char		delimiters[]	= "\n\t ,";
	uint16_t	address		= 0;

	if (output == NULL || input == NULL || list == NULL) {
		fprintf(stderr, "%s:%s: [!] Error: NULL parameter.\n",
				__FILE__, __func__);
		exit(EXIT_FAILURE);
	}
	
	memset(buffer_out, '\0', sizeof buffer_out);

	/* Walk through lines */
	while (fgets(buffer_in, sizeof buffer_in, input)) {
//		printf("Got line:\t%s", buffer_in);

		/* Walk through tokens */
		token = strtok(buffer_in, delimiters);
		while (token != NULL) {
//			printf("\tGot token:\t%s\n", token);

			/* Don't write the [label_name:] tokens to the
			 * output. They are already parsed. */
			if (is_label(token)) {
				token = strtok(NULL, delimiters);
				continue;
			}

			/* TODO(Alexander):
			 * 	Handle all possible stuff
			 */
			if (label_list_contains(list, token)) {
				sprintf(buffer_str, "0x%04x ",
					label_list_get_address(list, token));
			}
			else if (is_directive(token)	|| is_register(token) ||
				is_instruction(token)	|| is_binary(token) ||
				is_hex(token)) {
				sprintf(buffer_str, "%s ", token);
			}
			else {
				printf("[!] Compile error (line %d): Invalid "
					"token \"%s\".\n", address + 1, token);
				exit(EXIT_FAILURE);
			}

			strcat(buffer_out, buffer_str);

			/* Get next token */
			token = strtok(NULL, delimiters);
		}
		strcat(buffer_out, "\n");
		fprintf(output, "%s", buffer_out);
		memset(buffer_out, '\0', sizeof buffer_out);

		address += 1;
	}

	rewind(output);		/* A real programmer doesn't litter; */
	rewind(input);		/* "Reset" the read position in the file. */
}

void assemble_data(FILE* output, FILE* input)
{
	char		buffer_in[MAX_LINE_LENGTH];
	char*		token;			/* Part of an instruction */
	char*		delimiters;		/* Split tokens on these */
	uint16_t	lines_out[MEM_SIZE];
	uint16_t	line_nbr;		/* For compile error messages */
	uint16_t	data_size;		/* For the data header */

	delimiters	= "\t\n ";
	memset(lines_out, 0, sizeof *lines_out);
	line_nbr	= 0;
	data_size	= 0;

	while (fgets(buffer_in, sizeof buffer_in, input)) {

		line_nbr += 1;

		/* Get the first token if the line */
		token = strtok(buffer_in, delimiters);

		/* Skips empty lines */
		if (token == NULL) {
//			printf("Got NULL token.\n");
			continue;
		}

		/* Only parse .fill directives.
		 * TODO:
		 * 	Add the .space directive described in
		 * 	http://www.eng.umd.edu/~blj/RiSC/RiSC-isa.pdf
		 */
		if (!streq(token, ".fill")) {
			continue;
		}

		/* Get the next token */
		token = strtok(NULL, delimiters);

		if (token == NULL) {
			printf("[!] Compile error: Line %d: .fill directive is "
				"missing argument.\n", line_nbr);
			exit(EXIT_FAILURE);
		} 

		printf("token = %s\n", token);

		/* TODO:
		 * 	The programmer should be able to enter numbers such as
		 * 		290, -53, 0b1010101010101010, 0x32fc
		 */

		/* 
		 * Hex and binary checks
		 */
		if (token[0] == '0' && token[1] == 'x')
		{
			token += 2;
			printf("token = %s\n", token);
			if (strlen(token) != 4)
			{
				printf("token = %s\n", token);
				printf("[!] Compile error: Line %d: Hex numbers"
				" must be given as e.g. '0x1234'.\n", line_nbr);
				exit(EXIT_FAILURE);
			}
			if (!is_hex(token))
			{
				printf("[!] Compile error: Line %d: Invalid hex"
				" number 0x%s.\n", line_nbr, token);
				exit(EXIT_FAILURE);
			}
		}

		else if (token[0] == '0' && token[1] == 'b')
		{
			token += 2;
			if (strlen(token) != 16)
			{
				printf( "[!] Compile error: Line %d: binary "
				"numbers must be 16 characters long; is %lu "
				"characters long.\n", line_nbr, strlen(token));
				exit(EXIT_FAILURE);
			}
			if (!is_binary(token))
			{
				printf( "[!] Compile error: Line %d: Invalid "
				"binary number 0b%s.\n", line_nbr, token);
				exit(EXIT_FAILURE);
			}
		}

		else
		{
			printf("[!] Compile error: Line %d: Not a binary or hex"
				"adecimal number (%s).\n", line_nbr, token);
			exit(EXIT_FAILURE);
		}

		lines_out[data_size] = str_to_int(token);

		printf("hi\n");

		/* TODO(Alexander): Don't forget to copy the data to output! */
//		strcpy(lines_out[data_size], token);
		data_size += 1;
	}

	/* Print as binary */
	char binbuf[17];
	fprintf(output, "%s\n", dec_to_bin(binbuf, data_size, 16));
	for (int i = 0; i < data_size; ++i) {
		fprintf(output, "%s\n", dec_to_bin(binbuf, lines_out[i], 16));
	}

	/* Print as hex */
//	fprintf(output, "0x%04x\n", data_size);
//	for (int i = 0; i < data_size; ++i) {
//		fprintf(output, "0x%04x\n", lines_out[i]);
//	}

	rewind(output);		/* A real programmer doesn't litter; */
	rewind(input);          /* "Reset" the read position in the file. */
}

void assemble_text(FILE* output, FILE* input)
{
	char		buffer_in[MAX_LINE_LENGTH];
	uint16_t	lines_out[MEM_SIZE];
//	uint16_t	line_nbr = 0;
	uint16_t	text_size = 0;

	memset(lines_out, 0, sizeof *lines_out);
	while (fgets(buffer_in, sizeof buffer_in, input)) {
		assemble_line(&lines_out[text_size], &text_size, buffer_in);
//		text_size += 1;
	}

	/* Print as binary */
	char binbuf[17];
	fprintf(output, "%s\n", dec_to_bin(binbuf, text_size, 16));
	for (int i = 0; i < text_size; ++i) {
		fprintf(output, "%s\n",
				dec_to_bin(binbuf, lines_out[i], 16));
	}

	rewind(output);		/* A real programmer doesn't litter; */
	rewind(input);          /* "Reset" the read position in the file. */
}

/* Assembles an assembly line [src] and stores the result as an unsigned 16-bit
 * integer in [dest]
 */
static void assemble_line(uint16_t* dest, uint16_t* line_num, const char* src)
{
	char*	delimiters	= "\n\t ,";
	char**	tokens		= calloc(WORD_SIZE, sizeof(char*));
	int	num_tokens;	/* Used when calling the instructions */

	/* All of the below values are not certain to exist, namely arg3 */
	char	arg1	[11]	= {0};
	char	arg2	[11]	= {0};
	char	arg3	[11]	= {0};

	/* Split up line into tokens */
	num_tokens = tokenize(tokens, src, delimiters);	

	/* Parse the tokens */
	for (int i = 0; i < num_tokens; ++i) {

		if (tokens[i] == NULL) {
			printf("[!] Got NULL token\n");
			continue;
		}

		if (is_directive(tokens[i])) {
//			printf("__ DIRECTIVE __\n");
			break;
		}

		switch (num_tokens - 1) {
		case 3:	strcpy(arg3, tokens[i + 3]);
		case 2:	strcpy(arg2, tokens[i + 2]);
		case 1:	strcpy(arg1, tokens[i + 1]);
		}

		printf("Input:\t%s ", tokens[i]);
		for (int j = 1; j < num_tokens; ++j)
			printf("%s ", tokens[i + j]);
		printf("\n");


		char* t = tokens[i];

		uint16_t
		regA, regB, regC, simm, uimm;

		if (streq(t, "add" ) || streq(t, "nand")) {
			regA = get_reg_num(arg1) << 10;
			regB = get_reg_num(arg2) << 7;
			regC = get_reg_num(arg3);

		} else if (streq(t, "addi") || streq(t, "sw")
			|| streq(t, "lw") || streq(t, "lw")) {
			regA = get_reg_num(arg1) << 10;
			regB = get_reg_num(arg2) << 7;
			simm = str_to_int(arg3) & MASK_LOW_7;

		} else if (streq(t, "lui" )) {
			regA = get_reg_num(arg1) << 10;
			uimm = str_to_int(arg2) & MASK_LOW_10;

		} else if (streq(t, "jalr")) {
			regA = get_reg_num(arg1) << 10;
			regB = get_reg_num(arg2) << 7;
		}

		*dest = streq(t, "add" ) ?  0x0000 | regA | regB | regC :
			streq(t, "addi") ?  0x2000 | regA | regB | simm :
			streq(t, "nand") ?  0x4000 | regA | regB | regC :
			streq(t, "lui" ) ?  0x6000 | regA | uimm        :
			streq(t, "sw"  ) ?  0x8000 | regA | regB | simm :
			streq(t, "lw"  ) ?  0xa000 | regA | regB | simm :
			streq(t, "beq" ) ?  0xc000 | regA | regB | simm :
			streq(t, "jalr") ? (0xe000 | regA | regB) & MASK_UPP_9
			: 0xe001; /* The ternary operator needs an "else" */

		if (*dest == 0xe001) {
			// TODO(Alexander): What happens if it wasn't a label?
			printf("BIRD POOP!-----------------------------\n");
			fprintf(stderr, "[!] Compile error (line %u): Unknown "
					"opcode \"%s\".\n", *line_num, t);
			/// TODO
		}

//		uint16_t regA = get_reg_num(arg1) << 10;
//		uint16_t regB = get_reg_num(arg2) << 7;
//		uint16_t regC = get_reg_num(arg3);
//		uint16_t simm = hex_to_dec(arg3) & MASK_LOW_7;
//		uint16_t uimm = hex_to_dec(arg2) & MASK_LOW_10;
//
//		*dest = streq(t, "add" ) ?  0x0000 | regA | regB | regC :
//			streq(t, "addi") ?  0x2000 | regA | regB | simm :
//			streq(t, "nand") ?  0x4000 | regA | regB | regC :
//			streq(t, "lui" ) ?  0x6000 | regA | uimm        :
//			streq(t, "sw"  ) ?  0x8000 | regA | regB | simm :
//			streq(t, "lw"  ) ?  0xa000 | regA | regB | simm :
//			streq(t, "beq" ) ?  0xc000 | regA | regB | simm :
//			streq(t, "jalr") ? (0xe000 | regA | regB) & MASK_UPP_9
//			: 0xe001; /* The ternary operator needs an "else" */
//
//		if (*dest == 0xe001) {
//			// TODO(Alexander): What happens if it wasn't a label=
//		}

		char binbuf[17];
		printf("*dest = %s\n", dec_to_bin(binbuf, *dest, 16));

		*line_num += 1;

		break;	/* Unconditional break at the end of the for loop, why?
			   The function should only actually keep looping as
			   long as the first token is _not_ an instruction.
			   When the function finally encounters an instruction,
			   it counts the arguments and acts accordingly. */
	}

	if (tokens != NULL) {
		free(tokens);
	}
}

int tokenize(char** tokens, const char* src, const char* delimiters)
{
	char	src_copy[MAX];
	char*	token;
	int	num_tokens;

	strncpy		(src_copy, src, strlen(src) + 1);
	num_tokens	= 0;
	token		= strtok(src_copy, delimiters);

	while (token != NULL) {
		tokens[num_tokens] = token;
		num_tokens += 1;
		token = strtok(NULL, delimiters);
	}

	return num_tokens;
}
















#if 0
void assemble_text(FILE* output, FILE* input)
{
	char	out_buffer	[16 + 1];	/* Store assembled line here */
	char	in_buffer	[160 + 1];	/* 160 = Maximum line length */
	char*	out_lines	[1024];
	int	text_size	= 0;

	for (int i = 0; i < 1024; ++i) {
		out_lines[i] = malloc(16 + 1);
		if (out_lines[i] == NULL) {
			printf("[!] Error: Out of memory!\n");
			exit(EXIT_FAILURE);
		}
	}

	memset(out_buffer, '\0', sizeof out_buffer);
	while (fgets(in_buffer, sizeof in_buffer, input)) {
		assemble_line(out_buffer, in_buffer);

		if (streq(out_buffer, "")) {
			continue;
		}

		text_size += 1;

		strcpy(out_lines[text_size - 1], out_buffer);
		memset(out_buffer, '\0', sizeof out_buffer);
	}

	char bin_buffer[16 + 1];
	fprintf(output, "%s\n", dec_to_bin(bin_buffer, text_size, 16));

	for (int i = 0; i < 1024; ++i) {
		if (i < text_size) {
			fprintf(output, "%s\n", out_lines[i]);
		}
		free(out_lines[i]);
	}
	rewind(output);		/* A real programmer doesn't litter; */
	rewind(input);          /* "Reset" the read position in the file. */
}

static void assemble_line(char* dest, const char* src)
{
	char*	delimiters	= "\n\t ,";
	char**	tokens		= calloc(WORD_SIZE, sizeof(char*));
	int	nbr_tokens;	/* Used when calling the instructions */

	/* All of the below values are not certain to exist, namely arg3 */
	char	arg1	[11]	= {0};
	char	arg2	[11]	= {0};
	char	arg3	[11]	= {0};

	/* Split up line into tokens */
	nbr_tokens = tokenize(tokens, src, delimiters);	

	/* Parse the tokens */
	for (int i = 0; i < nbr_tokens; ++i) {

		if (tokens[i] == NULL) {
			printf("[!] Got NULL token\n");
			continue;
		}

		if (is_directive(tokens[i])) {
//			printf("__ DIRECTIVE __\n");
			break;
		}

		switch (nbr_tokens - 1) {
		case 0:
			printf("0 args\n");
			break;
		case 1:
			strcpy(arg1, tokens[i + 1]);
			printf("Input:\t%s %s\n", tokens[i], arg1);
			break;
		case 2:
			strcpy(arg1, tokens[i + 1]);
			strcpy(arg2, tokens[i + 2]);
			printf("Input:\t%s %s, %s\n", tokens[i], arg1, arg2);
			break;
		case 3:
			strcpy(arg1, tokens[i + 1]);
			strcpy(arg2, tokens[i + 2]);
			strcpy(arg3, tokens[i + 3]);
			printf("Input:\t%s %s, %s, %s\n",
					tokens[i], arg1, arg2, arg3);
			break;
		}

		char* t = tokens[i];

		if (streq(t, "add" ))	{ parse_add  (dest, arg1, arg2, arg3); }
		if (streq(t, "addi"))	{ parse_addi (dest, arg1, arg2, arg3); }
		if (streq(t, "nand"))	{ parse_nand (dest, arg1, arg2, arg3); }
		if (streq(t, "lui" ))	{ parse_lui  (dest, arg1, arg2); }
		if (streq(t, "sw"  ))	{ parse_sw   (dest, arg1, arg2, arg3); }
		if (streq(t, "lw"  )) 	{ parse_lw   (dest, arg1, arg2, arg3); }
		if (streq(t, "beq" ))	{ parse_beq  (dest, arg1, arg2, arg3); }
		if (streq(t, "jalr"))	{ parse_jalr (dest, arg1, arg2); }

		break;	/* Unconditional break at the end of the for loop, why?
			   The function should only actually keep looping as
			   long as the first token is _not_ an instruction.
			   When the function finally encounters an instruction,
			   it counts the arguments and acts accordingly. */

	}

	if (tokens != NULL) {
		free(tokens);
	}

	if (!streq(dest, "")) {
//		printf("Result:\t%s\n\n", dest);
	}
}
#endif

///* Searches for labels in [input] and stores them inside [list].
// */
//void parse_labels(FILE* input, label_list_t* list)
//{
//	char		line_buffer[160 + 1];
//	char		bin_buffer[16 + 1];	/* For the binary conversion */
//	char*		token;			/* Usage: find a label */
//	char*		delimiters;		/* Chars to split tokens on */
//	int		label_addr;
//
//	delimiters	= "\n\t ";
//	label_addr	= 0;
//
//	if (input == NULL || list == NULL) {
//		fprintf(stderr, "%s:%s: [!] Error: NULL parameter.\n",
//				__FILE__, __func__);
//		exit(EXIT_FAILURE);
//	}
//
//	while (fgets(line_buffer, sizeof line_buffer, input)) {
//
//		if (is_empty_line(line_buffer)) {
//			continue;
//		}
//
//		label_addr += 1;
//		token = strtok(line_buffer, delimiters);
//
////		// XXX(New)
////		// Won't work because this function parses the left side token.
////		if (streq(token, "lui")) {
////			printf("got_lui = true! <<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
////			got_lui = true;
////			
////			strtok(line_buffer, delimiters);
////			token = strtok(line_buffer, delimiters);
////		}
//
//		if (is_label(token)) {
//			token[strlen(token) - 1] = '\0';
//			label_list_add_label(
//				list,
//				token,
//				dec_to_bin(bin_buffer, label_addr, 7);
//			);
//		}
//	}
//	/* TODO: Remove this when done with this part. */
//	//label_list_print(list);
//
//	rewind(input);		/* A real programmer doesn't litter. */
//}


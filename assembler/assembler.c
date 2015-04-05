#include <ctype.h>
#include <inttypes.h>
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

void check_registers(FILE* input)
{
	char		buffer[MAX_LINE_LENGTH];
	char*		token;
	char		delimiters[] = "\t\n ,";
	uint16_t	line = 0;

	while (fgets(buffer, sizeof buffer, input)) {
		line += 1;
		token = strtok(buffer, delimiters);
		while (token != NULL) {
			if (token[0] == 'r' && isdigit(token[1])) {
				if (!is_register(token)) {
					printf("[!] Compile error (line %u): "
						"Invalid register \"%s\".\n",
						line, token);
					exit(EXIT_FAILURE);
				}
			}
			token = strtok(NULL, delimiters);
		}
	}
	rewind(input);
}

/* Parse Labels
 * 		If a label such as "example:" is found in [input], store the
 * 		address (which ranges from 0 to 2^16 - 1) inside [list].
 */
void parse_labels(FILE* input, label_list_t* list)
{
	char		buffer[MAX_LINE_LENGTH];
	char*		token;
	char*		next_token;
	char		delimiters[] = "\t\n ,";
	uint16_t	address	= 0;
	uint16_t	line	= 0;

	while (fgets(buffer, sizeof buffer, input)) {

		line += 1;

		if (is_empty_line(buffer))
			continue;

		token = strtok(buffer, delimiters); 

		if (strlast(token) == ':') {

			if (buffer[0] != token[0]) {
				printf("[!] Compile error (line %u): Labels may"
					" not be indented.\n", line);
				exit(EXIT_FAILURE);
			}

			/* Get next token. Should be directive or opcode. */
			next_token = strtok(NULL, delimiters);

			/* Remove ending ':' character */
			token[strlen(token) - 1] = '\0';

			if (next_token == NULL) {
				printf("[!] Compile error (line %u): Label \"%s"
					"\" may not be on a line by itself.\n",
					line, token);
				exit(EXIT_FAILURE);
			}

			/* Add 1 or 2 to the address, because the data will be
			 * prepended by a one-line data header and the text by
			 * a one-line text header
			 */
			/* TODO(Alexander): or .space */
			if (streq(next_token, ".fill")) {
				label_list_add(list, token, address + 1);

			} else if (is_instruction(next_token)) {
				label_list_add(list, token, address + 2);

			} else {
				printf("[!] Compile error (line %u): Unknown "
					"token \"%s\".\n", line, next_token);
				exit(EXIT_FAILURE);
			}
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
			if (strlast(token) == ':') {
				token = strtok(NULL, delimiters);
				continue;
			}

			char format[] = "0x%04x ";

			if (label_list_contains(list, token)) {
				sprintf(buffer_str, format,
					label_list_get_address(list, token));
				printf("&%s = %s\n", token, buffer_str);

			} else if (is_directive(token)	|| is_register(token) ||
				is_instruction(token)) {
				sprintf(buffer_str, "%s ", token);

			} else if (is_binary(token) || is_hex(token) ||
					is_dec(token)) {
				sprintf(buffer_str, format, str_to_int(token));

			} else {
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
		 * TODO(Alexander):
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

		/* 
		 * Hex and binary checks
		 */
		if (token[0] == '0' && token[1] == 'x')
		{
//			token += 2;
			printf("token = %s\n", token);
			if (strlen(token) != 2 + 4)
			{
				fprintf(stderr, "[!] Error in %s: %s.\n",
						__FILE__, __func__);
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

		data_size += 1;
	}

	/* Print as binary */
#if 0
	char binbuf[17];
	fprintf(output, "%s\n", dec_to_bin(binbuf, data_size, 16));
	for (int i = 0; i < data_size; ++i) {
		fprintf(output, "%s\n", dec_to_bin(binbuf, lines_out[i], 16));
	}
#endif

	/* Print as hex */
	char format[] = "0x%04x\n";
//	char format[] = "%"PRIu16"\n";
	fprintf(output, format, data_size);
	for (int i = 0; i < data_size; ++i) {
		fprintf(output, format, lines_out[i]);
	}

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
#if 0
	char binbuf[17];
	fprintf(output, "%s\n", dec_to_bin(binbuf, text_size, 16));
	for (int i = 0; i < text_size; ++i) {
		fprintf(output, "%s\n",
				dec_to_bin(binbuf, lines_out[i], 16));
	}
#endif

	/* Print as hex */
	char format[] = "0x%04x\n";
//	char format[] = "%"PRIu16"\n";
	fprintf(output, format, text_size);
	for (int i = 0; i < text_size; ++i) {
		fprintf(output, format, lines_out[i]);
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
			break;
		}

		switch (num_tokens - 1) {
		case 3:	strcpy(arg3, tokens[i + 3]);	/* Intentional */
		case 2:	strcpy(arg2, tokens[i + 2]);	/* fallthrough */	
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
			|| streq(t, "lw") || streq(t, "beq")) {
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
			fprintf(stderr, "----------------------------------\n");
			fprintf(stderr, "[!] Compile error (line %u): Unknown "
					"opcode \"%s\".\n", *line_num, t);
			fprintf(stderr, "----------------------------------\n");
		}

		char binbuf[17];
		printf("*dest = %s\n", dec_to_bin(binbuf, *dest, 16));
		printf("*dest = %"PRIu16"\n", *dest);
		printf("\n");

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



/* XXX
 * Currently unused functions
 */

#if 0
void file_remove_blank_lines(FILE* output, FILE* input)
{
	char buffer[160 + 1];

	while (fgets(buffer, sizeof buffer, input)) {
		if (!is_empty_line(buffer)) {
			fprintf(output, "%s", buffer);
		}
	}
}
#endif


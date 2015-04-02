#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "riscy.h"
#include "utility.h"

#define MAX		(1024)
#define WORD_SIZE	(16)

/* Splits up a line into proper assembly language tokens */
static int	tokenize	(char**		tokens,
				const char*	src,
				const char*	delimiters);

/* Assembles [src], a line with instructions, and stores the result in [dest] */
static void	assemble_line	(char* dest, const char* src);


/* Remove comments, trailing whitespace, and empty lines.
 * Store the result in a new file (output).
 */
/* XXX Edit: Does not remove blank lines anymore */
void file_cleanup(FILE* output, FILE* input)
{
	char buffer[160 + 1];

	while (fgets(buffer, sizeof buffer, input)) {
		remove_comments(buffer);
		remove_trailing_whitespace(buffer);
//		if (is_empty_line(buffer)) {	
//			continue;
//		}
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

/* Searches for labels in "input" and stores them inside "list".
 */
void parse_labels(FILE* input, label_list_t* list)
{
	char		line_buffer[160 + 1];
	char		bin_buffer[16 + 1];	/* For the binary conversion */
	char*		token;			/* Usage: find a label */
	char*		delimiters;		/* Chars to split tokens on */
	int		label_addr;

	delimiters	= "\n\t ";
	label_addr	= 0;

	if (input == NULL || list == NULL) {
		fprintf(stderr, "%s:%s: [!] Error: NULL parameter.\n",
				__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	while (fgets(line_buffer, sizeof line_buffer, input)) {

		if (is_empty_line(line_buffer)) {
			continue;
		}

		label_addr += 1;
		token = strtok(line_buffer, delimiters);

		if (is_label(token)) {
			token[strlen(token) - 1] = '\0';
			label_list_add_label(
				list,
				token,
				dec_to_bin(bin_buffer, label_addr, 7)
			);
		}
	}
	/* TODO: Remove this when done with this part. */
	//label_list_print(list);

	rewind(input);		/* A real programmer doesn't litter. */
}

/* Reduces the input by removing all labels "to the left", i.e. those that end
 * with a ':', and also replaces the lables given as arguments to instructions
 * with their binary representations, i.e. their real addresses.
 */
void replace_labels(FILE* output, FILE* input, label_list_t* list)
{
	char	in_buffer[160 + 1];	/* The line read from [input] */
	char	out_buffer[160 + 1];	/* The line to write to [output] */
	char	str_buffer[160 + 1];	/* Used with sprintf further down */
	char*	token;			/* Part of an instruction */
	char*	delimiters;		/* Split tokens on the delimiters */
	int	line_nbr;		/* For printing compile error message */

	if (output == NULL || input == NULL || list == NULL) {
		fprintf(stderr, "%s:%s: [!] Error: NULL parameter.\n",
				__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	memset(out_buffer, '\0', sizeof out_buffer);
	delimiters	= "\n\t ,";
	line_nbr	= 0;

	/* Walk through lines */
	while (fgets(in_buffer, sizeof in_buffer, input)) {
//		printf("Got line:\t%s", in_buffer);

		line_nbr += 1;

		/* Walk through tokens */
		token = strtok(in_buffer, delimiters);
		while (token != NULL) {
//			printf("\tGot token:\t%s\n", token);

			/* Don't write the [label_name:] tokens to the
			 * output. They are already parsed. */
			if (is_label(token)) {
				token = strtok(NULL, delimiters);
				continue;
			}

			if (label_list_contains(list, token)) {
				sprintf(str_buffer, "%s ",
					label_list_get_address(list, token));
			}
			else if (is_directive(token)	|| is_register(token) ||
				is_instruction(token)	|| is_binary(token)) {
				sprintf(str_buffer, "%s ", token);
			}
			else {
				printf("[!] Compile error (line %d): Invalid "
					"token \"%s\".\n", line_nbr, token);
				exit(EXIT_FAILURE);
			}

			strcat(out_buffer, str_buffer);

			/* Get next token */
			token = strtok(NULL, delimiters);
		}
		strcat(out_buffer, "\n");
		fprintf(output, "%s", out_buffer);
		memset(out_buffer, '\0', sizeof out_buffer);
	}
	rewind(output);		/* A real programmer doesn't litter; */
	rewind(input);		/* "Reset" the read position in the file. */
}

void assemble_data(FILE* output, FILE* input)
{
	char	in_buffer	[160 + 1];	/* 160 = Maximum line length */
	char*	token;				/* Part of an instruction */
	char*	delimiters;                     /* Split tokens on these */
	char*	out_lines	[1024];
	int	line_nbr;			/* For compile error messages */
	int	data_size;			/* For the data header */

	delimiters	= "\t\n ";
	line_nbr	= 0;
	data_size	= 0;

	/* Allocate memory for the lines that will be printed */
	for (int i = 0; i < 1024; ++i) {
		out_lines[i] = malloc(16 + 1);
		if (out_lines[i] == NULL) {
			printf("[!] Out of memory!\n");
			exit(EXIT_FAILURE);
		}
	}

	while (fgets(in_buffer, sizeof in_buffer, input)) {

		line_nbr += 1;

		/* Get the first token if the line */
		token = strtok(in_buffer, delimiters);

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

		/* Get the next token. Exit with compile error message UNLESS:
		 * 	- token is not NULL
		 * 	- token is exactly 16 characters long
		 * 	- token is a binary number */
		token = strtok(NULL, delimiters);

//		printf(">>>>>>>>> token = \"%s\"\n", token);

		if (token == NULL) {
			printf("[!] Compile error: Line %d: .fill directive is "
				"missing argument.\n", line_nbr);
			exit(EXIT_FAILURE);
		} 
		if (strlen(token) != 16) {
			printf("[!] Compile error: Line %d: .fill directive has"
				" invalid argument length; must be 16 bits; "
				"is %lu bits.\n", line_nbr, strlen(token));
			exit(EXIT_FAILURE);
		}
		if (!is_binary(token)) {
			printf("[!] Compile error: Line %d: .fill directive has"
				" invalid argument; must be a binary number.\n",
				line_nbr);
			exit(EXIT_FAILURE);
		}

		data_size += 1;

		strcpy(out_lines[data_size - 1], token);
	}

	char bin_buffer[16 + 1];
	fprintf(output, "%s\n", dec_to_bin(bin_buffer, data_size, 16));

	for (int i = 0; i < 1024; ++i) {
		if (i < data_size && !streq(out_lines[i], "")) {
			fprintf(output, "%s\n", out_lines[i]);
		}
		free(out_lines[i]);
	}
	rewind(output);		/* A real programmer doesn't litter; */
	rewind(input);          /* "Reset" the read position in the file. */
}

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
//			printf("0 args\n");
			break;
		case 1:
			strcpy(arg1, tokens[i + 1]);
//			printf("Input:\t%s %s\n", tokens[i], arg1);
			break;
		case 2:
			strcpy(arg1, tokens[i + 1]);
			strcpy(arg2, tokens[i + 2]);
//			printf("Input:\t%s %s, %s\n", tokens[i], arg1, arg2);
			break;
		case 3:
			strcpy(arg1, tokens[i + 1]);
			strcpy(arg2, tokens[i + 2]);
			strcpy(arg3, tokens[i + 3]);
//			printf("Input:\t%s %s, %s, %s\n",
//					tokens[i], arg1, arg2, arg3);
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

int tokenize(char** tokens, const char* src, const char* delimiters)
{
	char	src_copy[MAX];
	char*	token;
	int	nbr_tokens;

	strncpy(src_copy, src, strlen(src) + 1);
	token		= strtok(src_copy, delimiters);
	nbr_tokens	= 0;

	while (token != NULL) {
		tokens[nbr_tokens] = token;
		token = strtok(NULL, delimiters);
		++nbr_tokens;
	}

	return nbr_tokens;
}

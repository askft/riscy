/* main.c */

#include "assembler.h"
#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO(Alexander)
 * 	Search for all TODOs in assembler.c.
 */

int main(int argc, char* argv[])
{
	char*	input_filename	= argv[1];	/* Assembly code to assemble */
	char*	output_filename	= argv[2];	/* Output of assembled code */
	char*	tmp_filename1	= "fbf7c968";	/* Temporary buffer file 1 */
	char*	tmp_filename2	= "62fe98d2";	/* Temporary buffer file 2 */
	FILE*	input		= NULL;	
	FILE*	output		= NULL;		
	FILE*	tmpfile1	= NULL;
	FILE*	tmpfile2	= NULL;

	symtable_t*	symtable;		/* Will be assigned to the return value
					   of the parsed_labels function, to
					   later be passed as a parameter into
					   the replace_labels function.
					   [!!!] This has to be freed by calling
					   symtable_free. */

	if (argc != 3) {
		printf("Usage: assembler <input_filename> <output_filename>\n");
		exit(EXIT_FAILURE);
	}

	/* File must end with ".s" */
	char* ext = strrchr(input_filename, '.');
	if (ext == '\0') {
		fprintf(stderr, "Invalid filename \"%s\"; file must end with a "
				".s extension.\n", input_filename);
		exit(EXIT_FAILURE);
	} else if (!streq(".s", ext)) {
		fprintf(stderr, "Invalid extension \"%s\"; file must end with a"
				" .s extension.\n", ext);
		exit(EXIT_FAILURE);
	}

	printf( "\n========================================================\n"
		"Starting assembler.\n"
		"========================================================\n\n");

	/* Open files for [file_cleanup] */
	input		= safer_fopen(input_filename, "r");
	tmpfile1	= safer_fopen(tmp_filename1, "w");

	/* Make the file easier to parse by removing all comments,
	 * empty lines and whitespace */
	printf("Start : file_cleanup\n");
	file_cleanup(tmpfile1, input);
	printf("End   : file_cleanup\n\n");

	/* First temporary file should now be read from instead of written to */
	fclose(tmpfile1);
	tmpfile1 = safer_fopen(tmp_filename1, "r");

	printf("Start : check_register\n");
	check_registers(tmpfile1);
	printf("End   : check_register\n");

	/* Scan for labels and store their respective addresses */
	symtable = symtable_init();
	printf("Start : parse_labels\n");
	parse_labels(tmpfile1, symtable);
	printf("End   : parse_labels\n\n");

	/* Tedious to read from and write to the first temporary file at the
	 * same time. Open a second temporary file to write to instead. */
	tmpfile2 = safer_fopen(tmp_filename2, "w");

	/* Replace all labels with their binary addressed */
	printf("Start : replace_labels\n");
	replace_labels(tmpfile2, tmpfile1, symtable);
	printf("End   : replace_labels\n\n");

	/* Reopen tmpfile2 for reading, and open the user output file for
	 * writing. */
	fclose(tmpfile2);
	tmpfile2 = safer_fopen(tmp_filename2, "r");
	output = safer_fopen(output_filename, "w");

	/* Assemble the .fill directives to binary */
	printf("Start : assemble_data.\n");
	assemble_data(output, tmpfile2);
	printf("End   : assemble_data.\n\n");

	/* Close [output] for writing, repoen it for appending (the data). */
	fclose(output);
	output = safer_fopen(output_filename, "a");

	/* Assemble the instructions to binary */
	printf("Start : assemble_text.\n");
	assemble_text(output, tmpfile2);
	printf("End   : assemble_text.\n\n");

	/* Done with the assembly. Close all files and free all memory. */
	remove(tmp_filename1);
	remove(tmp_filename2);
	fclose(input);
	fclose(tmpfile1);
	fclose(tmpfile2);
	fclose(output);

	symtable_free(symtable);

	printf( "\n========================================================\n"
		"Exiting assembler.\n"
		"========================================================\n\n");

	exit(EXIT_SUCCESS);
}


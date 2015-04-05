#include <stdlib.h>

#include "assembler.h"
#include "utility.h"

/* TODO(Alexander)
 * 	Search for all TODOs in assembler.c.
 */

int main(int argc, char* argv[])
{
	char*	input_filename	= argv[1];	/* Assembly code to assemble */
	char*	output_filename	= argv[2];	/* Output of assembled code */
	char*	tmp_filename1	= "tmp/tmp1.s";	/* Temporary buffer file 1 */
	char*	tmp_filename2	= "tmp/tmp2.s";	/* Temporary buffer file 2 */
//	char*	tmp_filename3	= "tmp3.o";
	FILE*	input		= NULL;	
	FILE*	output		= NULL;		
	FILE*	tmpfile1	= NULL;
	FILE*	tmpfile2	= NULL;
//	FILE*	tmpfile3	= NULL;

	label_list_t*	list;		/* Will be assigned to the return value
					   of the parsed_labels function, to
					   later be passed as a parameter into
					   the replace_labels function.
					   [!!!] This has to be freed by calling
					   label_list_free. */

	if (argc != 3) {
		printf("Usage: assembler <input_filename> <output_filename>\n");
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

	/* Scan for labels and store their respective addresses */
	list = label_list_init();
	printf("Start : parse_labels\n");
	parse_labels(tmpfile1, list);
	printf("End   : parse_labels\n\n");

	/* Tedious to read from and write to the first temporary file at the
	 * same time. Open a second temporary file to write to instead. */
	tmpfile2 = safer_fopen(tmp_filename2, "w");

	/* Replace all labels with their binary addressed */
	printf("Start : replace_labels\n");
	replace_labels(tmpfile2, tmpfile1, list);
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
	printf("Start : assemble_test.\n");
	assemble_text(output, tmpfile2);
	printf("End   : assemble_text.\n\n");

	/* Remove all empty lines */
//	remove_empty_lines(

	/* Done with the assembly. Close all files and free all memory. */
	fclose(input);
	fclose(tmpfile1);
	fclose(tmpfile2);
	fclose(output);

	label_list_free(list);

	printf( "\n========================================================\n"
		"Exiting assembler.\n"
		"========================================================\n\n");

	exit(EXIT_SUCCESS);
}


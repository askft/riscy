#include <stdlib.h>

#include "assembler.h"
#include "utility.h"

int main(int argc, char* argv[])
{
	char*	input_filename	= argv[1];	/* Assembly code to assemble */
	char*	output_filename	= argv[2];	/* Output of assembled code */
	char*	tmp_filename1	= "tmp1.s";	/* Temporary buffer file 1 */
	char*	tmp_filename2	= "tmp2.s";	/* Temporary buffer file 2 */
	FILE*	input		= NULL;	
	FILE*	output		= NULL;		
	FILE*	tmpfile1	= NULL;
	FILE*	tmpfile2	= NULL;

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
	printf("Cleaning up input file...\n");
	printf("%s\n", input_filename);
	file_cleanup(tmpfile1, input);
	printf("Successfully cleaned up input file.\n");

	/* First temporary file should now be read from instead of written to */
	fclose(tmpfile1);
	tmpfile1 = safer_fopen(tmp_filename1, "r");

	/* Scan for labels and store their respective addresses */
	printf("Parsing labels...\n");
	list = label_list_init();
	parse_labels(tmpfile1, list);
	printf("Successfully parsed labels.\n");

	/* Tedious to read from and write to the first temporary file at the
	 * same time. Open a second temporary file to write to instead. */
	tmpfile2 = safer_fopen(tmp_filename2, "w");

	/* Replace all labels with their binary addressed */
	printf("Replacing labels...\n");
	replace_labels(tmpfile2, tmpfile1, list);
	printf("Successfully replaced labels.\n\n");

	/* Reopen tmpfile2 for reading, and open the user output file for
	 * writing. */
	fclose(tmpfile2);
	tmpfile2 = safer_fopen(tmp_filename2, "r");
	output = safer_fopen(output_filename, "w");

	/* Assemble the .fill directives to binary */
	assemble_data(output, tmpfile2);

	/* Close [output] for writing, repoen it for appending (the data). */
	fclose(output);
	output = safer_fopen(output_filename, "a");

	/* Assemble the instructions to binary */
	assemble_text(output, tmpfile2);
	
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


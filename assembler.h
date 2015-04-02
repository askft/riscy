#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "stdio.h"
#include "riscy.h"

/* Removes comments, trailing whitespace and empty lines */
void	file_cleanup	(FILE* output, FILE* input);

/* Search for labels in "input" and stores them in "list" */
void	parse_labels	(FILE* input, label_list_t* list);

/* Puts the binary representations of the labels in [output], as a replacement
 * of the symbolic labels in [input]. */
void	replace_labels	(FILE* output, FILE* input, label_list_t* list);

/* Converts a "clean" file with assembly instructions to ones and zeros */

void	assemble_data	(FILE* output, FILE* input);
void	assemble_text	(FILE* output, FILE* input);

#endif


#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "symtable.h"

#include <stdio.h>

/* PASSES
 * Pass 1:	Remove comments and trailing whitespace.
 * Pass 2:	Check so that there are valid registers.
 * Pass 3:	Store the addresses of all labels that end with ':'.
 * Pass 4:	Replace labels with their addresses from pass 2.
 * 		Also report symbol errors.
 * Pass 5:	Assemble data.
 * Pass 6:	Assemble text.
 */

/* Removes comments and trailing whitespace */
void	file_cleanup	(FILE* output, FILE* input);

/* Check so that there are no tokens that begin with 'r' and end with one or
 * more digits. */
void	check_registers	(FILE* input);

/* Search for labels in `input` and stores them in `symtable` */
void	parse_labels	(FILE* input, symtable_t* symtable);

/* Puts the binary representations of the labels in [output], as a replacement
 * of the symbolic labels in [input]. */
void	replace_labels	(FILE* output, FILE* input, symtable_t* symtable);

/* Converts a "clean" file with assembly instructions to ones and zeros */
void	assemble_data	(FILE* output, FILE* input);
void	assemble_text	(FILE* output, FILE* input);

#endif


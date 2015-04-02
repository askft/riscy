/* utility.h */

#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>
#include <stdio.h>

FILE*	safer_fopen			(char* filename, char* action);

char*	dec_to_bin			(char* bin, int dec, int nbr_bits);
bool	is_binary			(const char* str);

void	remove_comments			(char* line);
void	remove_trailing_whitespace	(char* line);

char	strlast				(const char* str);
bool	is_instruction			(const char* str);
bool	is_empty_line			(const char* str);
bool	is_comment_line			(const char* str);
bool	is_directive			(const char* str);
bool	is_label			(const char* str);
bool	is_comment			(const char* str);
bool	is_register			(const char* str);
bool	streq				(const char* s1, const char* s2);

#endif


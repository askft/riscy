/* utility.h */

#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define MEM_SIZE		(0xffff)
#define NBR_REGISTERS		(8)
#define NBR_INSTRUCTIONS	(8)

FILE*	safer_fopen			(char* filename, char* action);

uint16_t	get_reg_num		(const char* name);
uint16_t	str_to_int		(const char* str);
char*		dec_to_bin		(char* bin, int dec, int nbr_bits);

void	remove_comments			(char* line);
void	remove_trailing_whitespace	(char* line);

char	strlast				(const char* str);
bool	streq				(const char* s1, const char* s2);
bool	is_dec				(const char* str);
bool	is_binary			(const char* str);
bool	is_hex				(const char* str);
bool	is_instruction			(const char* str);
bool	is_empty_line			(const char* str);
bool	is_comment_line			(const char* str);
bool	is_directive			(const char* str);
bool	is_label			(const char* str);
bool	is_comment			(const char* str);
bool	is_register			(const char* str);

#endif


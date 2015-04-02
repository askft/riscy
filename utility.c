/* utility.c */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "riscy.h"
#include "utility.h"


FILE* safer_fopen(char* filename, char* action)
{
	FILE* tmp = fopen(filename, action);
	if (tmp == NULL) {
		if (strcmp(action, "r") == 0)
			printf("[!] Failed to open file %s for reading.\n",
					filename);
		if (strcmp(action, "w") == 0)
			printf("[!] Failed to open file %s for writing.\n",
					filename);
		exit(EXIT_FAILURE);
	}
	return tmp;
}

char* dec_to_bin(char* bin, int dec, int nbr_bits)
{
	int i;
	bin[nbr_bits] = '\0';
	for (i = nbr_bits - 1; i >= 0; --i, dec >>= 1) {
		bin[i] = (dec & 1) + '0';
	}
	return bin;
}

bool is_binary(const char* str)
{
	unsigned long i;
	for (i = 0; i < strlen(str); ++i) {
		if (str[i] != '0' && str[i] != '1') {
			return false;
		}
	}
	return true;
}

void remove_comments(char* line)
{
	int i;
	for (i = 0; line[i] != '\0'; ++i) {
		if (line[i] == '#') {
			break;
		}
	}
	line[i]		= '\n';
	line[i + 1]	= '\0';
}

void remove_trailing_whitespace(char* line)
{
	char* end = line + strlen(line) - 1;

	while (isspace(*end)) {
		*end = '\0';
		--end;
	}
	*(end + 1) = '\n';
	*(end + 2) = '\0';
}

char strlast(const char* str)
{
	return str[strlen(str) - 1];
}

bool is_instruction(const char* str)
{
	for (int i = 0; i < NBR_INSTRUCTIONS; ++i) {
		if (streq(str, instructions[i])) {
			return true;
		}
	}
	return false;
}

bool is_empty_line(const char* str)
{
	while (*str != '\0') {
		if (!isspace(*str)) {
			return false;
		}
		str++;
	}
	return true;
}

bool is_comment_line(const char* str)
{
	while (isspace(*str))
		++str;
	return (*str == '#');
}

bool is_directive(const char* str)
{
	return str[0] == '.';
}

bool is_label(const char* str) {
	return strlast(str) == ':';
}

bool is_comment(const char* str)
{
	return str[0] == '#';
}

bool is_register(const char* str)
{
	return str[0] == 'r';
}

bool streq(const char* s1, const char* s2)
{
	return strcmp(s1, s2) == 0;
}


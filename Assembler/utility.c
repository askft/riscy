/* utility.c */

#include "utility.h"
#include "symtable.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

const char* instructions[NBR_INSTRUCTIONS] = {
	"add", "addi", "nand", "lui", "sw", "lw", "beq", "jalr",
};

FILE* safer_fopen(char* filename, char* action)
{
	FILE* tmp = fopen(filename, action);
	if (tmp == NULL) {
		if (strcmp(action, "r") == 0) {
			fprintf(stderr, "Failed to open %s for reading (%s)\n",
					filename, strerror(errno));
		}
		if (strcmp(action, "w") == 0) {
			fprintf(stderr, "Failed to open %s for writing (%s)\n",
					filename, strerror(errno));
		}
		exit(EXIT_FAILURE);
	}
	return tmp;
}

uint16_t get_reg_num(const char* reg)
{
	if (strlen(reg) != 2) {
		printf("[!] Compile error: Invalid token [%s].\n", reg);
		exit(EXIT_FAILURE);
	}
	return reg[1] - '0';
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

uint16_t str_to_int(const char* str)
{
	uint16_t ret;

	if (str[0] == '0' && str[1] == 'b') {
		ret = strtol((str + 2), NULL, 2);

	} else if (str[0] == '0' && str[1] == 'x') {
		ret = strtol(str, NULL, 16);

	} else {
		ret = strtol(str, NULL, 10);
	}

	return (uint16_t) ret;
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

bool streq(const char* s1, const char* s2)
{
	return strcmp(s1, s2) == 0;
}

bool is_dec(const char* str)
{
	bool has_sign = str[0] == '-' || str[0] == '+';
	if (has_sign && strlen(str) == 1) {
		return false;
	}
	for (unsigned long i = has_sign ? 1 : 0; i < strlen(str); ++i) {
		if (!isdigit(str[i])) {
			return false;
		}
	}
	return true;
}

bool is_binary(const char* str)
{
	if (strlen(str) <= 2) {
		return false;
	}
	if (!(str[0] == '0' && str[1] == 'b')) {
		return false;
	}
	for (unsigned long i = 2; i < strlen(str); ++i) {
		if ( ! (str[i] == '0' || str[i] == '1') ) {
			return false;
		}
	}
	return true;
}

bool is_hex(const char* str)
{
	if (strlen(str) <= 2) {
		return false;
	}
	if (!(str[0] == '0' && str[1] == 'x')) {
		return false;
	}
	for (unsigned long i = 2; i < strlen(str); ++i) {
		if ( ! (isdigit(str[i]) || ('a' <= str[i] && str[i] <= 'f')) ) {
			return false;
		}
	}
	return true;
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

/* TODO(Alexander):
 * 	Look for .fill or .space instead?
 */
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
	return 	str[0] == 'r'
		&& '0' <= str[1]
		&& str[1] <= '7'
		&& strlen(str) == 2;
}


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utility.h"

bool streq(const char* s1, const char* s2)
{
	return strcmp(s1, s2) == 0;
}

bool is_empty_line(const char* s)
{
	while (*s != '\0') {
		if (!isspace(*s)) {
			return false;
		}
		s++;
	}
	return true;
}

bool is_comment_line(const char* line)
{
	int i = 0;
	while (isspace(line[i]))
		++i;
	return line[i] == '#';
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

void remove_empty_lines(FILE* output, FILE* input)
{
	char	buffer	[160 + 1];

	while (fgets(buffer, sizeof buffer, input)) {
		if (!(is_empty_line(buffer) || is_comment_line(buffer))) {

			remove_comments(buffer);
			remove_trailing_whitespace(buffer);
			fputs(buffer, output);
		}
	}
}

int bin_to_dec(char* bin)
{
	return (int) strtol(bin, NULL, 2);
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

int main(int argc, char* argv[])
{
	printf("1111111111111111 to decimal = %d\n", bin_to_dec("1111111111111111"));
	// Binary test
//	if (is_binary("0a"))	printf("OK.\n"); else printf("Not OK.\n");
//	if (is_binary("000"))	printf("OK.\n"); else printf("Not OK.\n");
//	if (is_binary("1"))	printf("OK.\n"); else printf("Not OK.\n");
//	if (is_binary("1113"))	printf("OK.\n"); else printf("Not OK.\n");
//	if (is_binary("010"))	printf("OK.\n"); else printf("Not OK.\n");
//	if (is_binary("101"))	printf("OK.\n"); else printf("Not OK.\n");
//	if (is_binary("1010"))	printf("OK.\n"); else printf("Not OK.\n");
//	printf("Done.\n");

//	if (argc != 3) {
//		printf("Usage: test <input> <output>\n");
//		exit(1);
//	}
//
//	printf("input  = %s\n", argv[1]);
//	printf("output = %s\n", argv[2]);
//
//	FILE* input;
//	FILE* output;
//
//	input	= fopen(argv[1], "r");
//	output	= fopen(argv[2], "w");
//
//	remove_empty_lines(output, input);
//
//	fclose(input);
//	fclose(output);

	return 0;
}

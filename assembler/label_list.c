/* label_list.c */

#include "label_list.h"
#include "utility.h"

#include <stdlib.h>
#include <string.h>

const char* instructions[NBR_INSTRUCTIONS] = {
	"add", "addi", "nand", "lui", "sw", "lw", "beq", "jalr",
};

label_list_t* label_list_init()
{
	label_list_t*	list;
	
	list = malloc(sizeof *list);
	if (list == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: Failed to allocate memory.\n",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	list->nbr_labels = 0;
	for (int i = 0; i < MAX_LABELS; ++i) {
		list->labels[i] = NULL;
	}

	return list;
}

void label_list_add(label_list_t* list, char* name, uint16_t address)
{
	label_t*	label;

	if (list == NULL || name == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: NULL parameter.",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	label = malloc(sizeof *label);
	if (label == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: Failed to allocate memory.\n",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	strcpy(label->name, name);
	label->address = address;

	list->labels[list->nbr_labels++] = label;
}

uint16_t label_list_get_address(label_list_t* list, char* name)
{
	for (int i = 0; i < list->nbr_labels; ++i) {
		if (streq(name, list->labels[i]->name)) {
			return list->labels[i]->address;
		}
	}
	printf("[!] %s: %s: Could not find label \"%s\".\n",
		__FILE__, __func__, name);
	exit(EXIT_FAILURE);
}

bool label_list_contains(label_list_t* list, char* name)
{
	for (int i = 0; i < list->nbr_labels; ++i) {
		if (streq(name, list->labels[i]->name)) {
			return true;
		}
	}
	return false;
}

void label_list_print(label_list_t* list)
{
	if (list == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: NULL parameter.",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	printf("Printing labels:\n");
	for (int i = 0; i < list->nbr_labels; ++i) {
		printf(
			"&%s\t= 0x%04x\n",
			list->labels[i]->name,
			list->labels[i]->address
		);
	}
}

void label_list_free(label_list_t* list)
{
	for (int i = 0; i < list->nbr_labels; ++i) {
		if (list->labels[i] != NULL) {
			free(list->labels[i]);
		}
	}
	if (list != NULL) {
		free(list);
	}
}

uint16_t get_reg_num(const char* reg)
{
	if (strlen(reg) != 2) {
		printf("[!] Compile error: Invalid token [%s].\n", reg);
		exit(EXIT_FAILURE);
	}
	return reg[1] - '0';
}

#if 0
char* get_reg_bin(const char* name)
{
	if (streq(name, "r0"))	return "000";
	if (streq(name, "r1"))	return "001";
	if (streq(name, "r2"))	return "010";
	if (streq(name, "r3"))	return "011";
	if (streq(name, "r4"))	return "100";
	if (streq(name, "r5"))	return "101";
	if (streq(name, "r6"))	return "110";
	if (streq(name, "r7"))	return "111";

	fprintf(stderr, "[!] Invalid register name: %s\n", name);
	return NULL;
}
#endif


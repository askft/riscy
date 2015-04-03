/* riscy.c */

#include <stdlib.h>
#include <string.h>

#include "riscy.h"
#include "utility.h"

const char* instructions[NBR_INSTRUCTIONS] = {
	"add", "addi", "nand", "lui", "sw", "lw", "beq", "jalr",
};

//unsigned int memory[MEM_SIZE] = { 0 };
//unsigned int mempos_text = 0;

//void memstore_text(unsigned int data)
//{
//	
//}

label_list_t* label_list_init()
{
	int		i;
	label_list_t*	list;
	
	list = malloc(sizeof(label_list_t));
	if (list == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: Failed to allocate memory.\n",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	list->nbr_labels = 0;
	for (i = 0; i < MAX_LABELS; ++i) {
		list->labels[i] = NULL;
	}

	return list;
}

void label_list_add_label(label_list_t* list, char* name, char* address)
{
	label_t*	label;

	if (list == NULL || name == NULL || address == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: NULL parameter.",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	label = malloc(sizeof(label_t));
	if (label == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: Failed to allocate memory.\n",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	strcpy(label->name, name);
	strcpy(label->address, address);

	list->labels[list->nbr_labels++] = label;
}

char* label_list_get_address(label_list_t* list, char* name)
{
	for (int i = 0; i < list->nbr_labels; ++i) {
		if (streq(name, list->labels[i]->name)) {
			return list->labels[i]->address;
		}
	}
	printf("[!] %s: %s: Could not find label \"%s\".\n",
		__FILE__, __func__, name);
	return NULL;
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
	int i;

	if (list == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: NULL parameter.",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	printf("Printing labels:\n");
	for (i = 0; i < list->nbr_labels; ++i) {
		printf(
			"%s:\t%s\n",
			list->labels[i]->name,
			list->labels[i]->address
		);
	}
}

void label_list_free(label_list_t* list)
{
	// free shit here
}

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

void parse_add(char* str, char* arg1, char* arg2, char* arg3)
{
	strcat(str, "000");
	strcat(str, get_reg_bin(arg1));
	strcat(str, get_reg_bin(arg2));
	strcat(str, "0000");
	strcat(str, get_reg_bin(arg3));
}

void parse_addi(char* str, char* arg1, char* arg2, char* arg3)
{
	strcat(str, "001");
	strcat(str, get_reg_bin(arg1));
	strcat(str, get_reg_bin(arg2));
	strcat(str, arg3);
}

void parse_nand(char* str, char* arg1, char* arg2, char* arg3)
{
	strcat(str, "010");
	strcat(str, get_reg_bin(arg1));
	strcat(str, get_reg_bin(arg2));
	strcat(str, "0000");
	strcat(str, get_reg_bin(arg3));
}

void parse_lui(char* str, char* arg1, char* arg2)
{
	strcat(str, "011");
	strcat(str, get_reg_bin(arg1));
	strcat(str, arg2);
}

void parse_sw(char* str, char* arg1, char* arg2, char* arg3)
{
	strcat(str, "100");
	strcat(str, get_reg_bin(arg1));
	strcat(str, get_reg_bin(arg2));
	strcat(str, arg3);
}

void parse_lw(char* str, char* arg1, char* arg2, char* arg3)
{
	strcat(str, "101");
	strcat(str, get_reg_bin(arg1));
	strcat(str, get_reg_bin(arg2));
	strcat(str, arg3);
}

void parse_beq(char* str, char* arg1, char* arg2, char* arg3)
{
	strcat(str, "110");
	strcat(str, get_reg_bin(arg1));
	strcat(str, get_reg_bin(arg2));
	strcat(str, arg3);
}

void parse_jalr(char* str, char* arg1, char* arg2)
{
	strcat(str, "111");
	strcat(str, get_reg_bin(arg1));
	strcat(str, get_reg_bin(arg2));
	strcat(str, "0000000");
}


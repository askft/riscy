/* label_list.c */

#include "label_list.h"
#include "utility.h"

#include <stdlib.h>
#include <string.h>

#define MAX_LABELS		(MEM_SIZE)
#define MAX_LABEL_LENGTH	(80)

typedef struct label_t label_t;

struct label_list_t {
	uint16_t	nbr_labels;
	label_t*	labels[MAX_LABELS];
};

struct label_t {
	char		name[MAX_LABEL_LENGTH + 1];
	uint16_t	address;
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


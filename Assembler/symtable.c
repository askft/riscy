/* symtable.c */

#include "symtable.h"
#include "utility.h"

#include <stdlib.h>
#include <string.h>

#define MAX_LABELS		(MEM_SIZE)
#define MAX_LABEL_LENGTH	(80)

typedef struct entry_t entry_t;

struct symtable_t {
	uint16_t	nbr_entries;
	entry_t*	entries[MAX_LABELS];
};

struct entry_t {
	char		name[MAX_LABEL_LENGTH + 1];
	uint16_t	address;
};

symtable_t* symtable_init()
{
	symtable_t*	symtable;
	
	symtable = malloc(sizeof *symtable);
	if (symtable == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: Failed to allocate memory.\n",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	symtable->nbr_entries = 0;
	for (int i = 0; i < MAX_LABELS; ++i) {
		symtable->entries[i] = NULL;
	}

	return symtable;
}

void symtable_add(symtable_t* symtable, char* name, uint16_t address)
{
	entry_t*	entry;

	if (symtable == NULL || name == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: NULL parameter.",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	entry = malloc(sizeof *entry);
	if (entry == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: Failed to allocate memory.\n",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	strcpy(entry->name, name);
	entry->address = address;

	symtable->entries[symtable->nbr_entries++] = entry;
}

uint16_t symtable_get_address(symtable_t* symtable, char* name)
{
	for (int i = 0; i < symtable->nbr_entries; ++i) {
		if (streq(name, symtable->entries[i]->name)) {
			return symtable->entries[i]->address;
		}
	}
	printf("[!] %s: %s: Could not find label \"%s\".\n",
		__FILE__, __func__, name);
	exit(EXIT_FAILURE);
}

bool symtable_contains(symtable_t* symtable, char* name)
{
	for (int i = 0; i < symtable->nbr_entries; ++i) {
		if (streq(name, symtable->entries[i]->name)) {
			return true;
		}
	}
	return false;
}

void symtable_print(symtable_t* symtable)
{
	if (symtable == NULL) {
		fprintf(stderr,
			"%s:%s: [!] Error: NULL parameter.",
			__FILE__, __func__);
		exit(EXIT_FAILURE);
	}

	printf("Printing entries:\n");
	for (int i = 0; i < symtable->nbr_entries; ++i) {
		printf(
			"&%s\t= 0x%04x\n",
			symtable->entries[i]->name,
			symtable->entries[i]->address
		);
	}
}

void symtable_free(symtable_t* symtable)
{
	for (int i = 0; i < symtable->nbr_entries; ++i) {
		if (symtable->entries[i] != NULL) {
			free(symtable->entries[i]);
		}
	}
	if (symtable != NULL) {
		free(symtable);
	}
}


/**
 * symtable.h
 *
 * A symbol table in which symbolic labels found in the assembly program are
 * stored. This table only consist of pairs: the name of the symbol, and its
 * address.
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct symtable_t symtable_t;

/**
 * symtable_init
 * 	Creates a new symbol table and returns a pointer to it.
 * 	The pointer must be freed; this is done by calling symtable_free(ptr).
 */
symtable_t* symtable_init ();

/**
 * symtable_add
 * 	Adds an entry to the symbol table pointer to by `symtable`.
 * 	@param `symtable`	A pointer to the symbol table under operation.
 * 	@param `name`		The actual name of the symbol, e.g. "loop_1"
 * 	@param `address`	The address at which the symbol was found.
 */
void symtable_add (symtable_t* symtable, char* name, uint16_t address);

/**
 * symtable_get_address
 * 	Searches for a symbol string in the symbol table, and returns its
 * 	corresponding address if it was found. If the symbol was not present in
 * 	the table, exits the program.
 * 	TODO(Alexander): Is that really a good idea...?
 * 	@param `symtable`	A pointer the the symbol table under operation.
 * 	@param `name`		The symbol to search for.
 */
uint16_t symtable_get_address (symtable_t* symtable, char* name);

/**
 * symtable_contains
 * 	Returns whether or not the symbol table contains the symbol specified.
 * 	@param `symtable`	A pointer to the symbol table under operation.
 * 	@param `name`		The symbol to check for existence.
 */
bool symtable_contains (symtable_t* symtable, char* name);

/**
 * sumtable_print
 * 	Prints the entire symbol table as follows:
 * 		sym1    = 0x1234
 * 		test5	= 0x5678
 * 		memed	= 0x1337
 * 	@param `symtable`	A pointer to the symbol table under operation.
 */
void symtable_print (symtable_t* symtable);

/**
 * symtable_free
 * 	Deconstructs a symbol table created with symtable_init.
 *	@param `symtable`	A pointer to the symbol table under operation.
 */
void symtable_free (symtable_t* symtable);

#endif


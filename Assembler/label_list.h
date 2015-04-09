/* label_list.h */
/* Might also be called "symbol table" */

#ifndef LABEL_LIST_H
#define LABEL_LIST_H

#include <stdint.h>
#include <stdbool.h>

typedef struct	label_list_t	label_list_t;

label_list_t*	label_list_init		();
void		label_list_add		(label_list_t*	list,
					 char*		name,
					 uint16_t	address);
uint16_t	label_list_get_address	(label_list_t*, char* name);
bool		label_list_contains	(label_list_t* list, char* name);
void		label_list_print	(label_list_t* list);
void		label_list_free		(label_list_t* list);

#endif


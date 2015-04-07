/* label_list.h */

#ifndef LABEL_LIST_H
#define LABEL_LIST_H

#include <stdint.h>
#include <stdbool.h>

#define NBR_INSTRUCTIONS	(8)
#define NBR_REGISTERS		(8)
#define MEM_SIZE		(0xffff)
#define MAX_LABELS		(MEM_SIZE)
#define MAX_LABEL_LENGTH	(80)

extern const char*	instructions[NBR_INSTRUCTIONS];

typedef struct label_list_t	label_list_t;
typedef struct label_t		label_t;

struct label_list_t {
	int		nbr_labels : 10;	/* Max of 2^10 labels */
	label_t*	labels[MAX_LABELS];
};

struct label_t {
	char		name	[MAX_LABEL_LENGTH + 1];
	uint16_t	address;
};

label_list_t*	label_list_init		();
void		label_list_add		(	label_list_t*	list,
						char*		name,
						uint16_t	address);

uint16_t	label_list_get_address	(label_list_t*, char* name);
bool		label_list_contains	(label_list_t* list, char* name);
void		label_list_print	(label_list_t* list);
void		label_list_free		(label_list_t* list);

uint16_t	get_reg_num	(const char* name);

#if 0
char* get_reg_bin(const char* name);
#endif

#endif


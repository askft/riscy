/* riscy.h */

#ifndef RISCY_H
#define RISCY_H

#include <stdbool.h>

#define NBR_INSTRUCTIONS	(8)
#define NBR_REGISTERS		(8)
#define MEM_SIZE		(1024)
#define MAX_LABELS		(MEM_SIZE)	/* There are 2^10, i.e. 1024,
						   memory addresses. Each
						   address should be able to
						   have a unique label. */

extern const char*	instructions	[NBR_INSTRUCTIONS];
//extern unsigned int	memory		[MEM_SIZE];

/* Labels */

typedef struct label_list_t	label_list_t;
typedef struct label_t		label_t;

struct label_list_t {
	int		nbr_labels : 10;	/* Max of 2^10 labels */
	label_t*	labels[MAX_LABELS];
};

struct label_t {
	char		name	[80 + 1];
	char		address	[16 + 1];
};

label_list_t*	label_list_init		();
void		label_list_add_label	(	label_list_t*	list,
						char*		name,
						char*		address);

char*		label_list_get_address	(label_list_t*, char* name);
bool		label_list_contains	(label_list_t* list, char* name);
void		label_list_print	(label_list_t* list);
void		label_list_free		(label_list_t* list);


char*	get_reg_bin	(const char* name);

void	parse_add	(char* str, char* arg1, char* arg2, char* arg3	);
void	parse_addi	(char* str, char* arg1, char* arg2, char* arg3	);
void	parse_nand	(char* str, char* arg1, char* arg2, char* arg3	);
void	parse_lui	(char* str, char* arg1, char* arg2		);
void	parse_sw	(char* str, char* arg1, char* arg2, char* arg3	);
void	parse_lw	(char* str, char* arg1, char* arg2, char* arg3	);
void	parse_beq	(char* str, char* arg1, char* arg2, char* arg3	);
void	parse_jalr	(char* str, char* arg1, char* arg2		);

#endif


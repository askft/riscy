#ifndef VM_H
#define VM_H

#include <stdbool.h>
#include <stdint.h>

#define MEMORY_SIZE		(0x0000ffff)
#define WORD_SIZE		(16)
#define NUM_REGISTERS		(8)

typedef struct	RiscyVM		RiscyVM;
typedef struct	instruction_t	instruction_t;

RiscyVM*	VM_init		(char filename[]);
void		VM_shutdown	(RiscyVM* vm);
bool		VM_is_running	(RiscyVM* vm);

void		VM_fetch	(RiscyVM* vm);
void		VM_decode	(RiscyVM* vm);
void		VM_execute	(RiscyVM* vm);


#endif


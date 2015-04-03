#ifndef VM_H
#define VM_H

#include <stdbool.h>
#include <stdint.h>

#define MEMORY_SIZE		(0x0000ffff)
#define WORD_SIZE		(16)
#define NUM_REGISTERS		(8)

typedef struct RiscyVM	RiscyVM;

RiscyVM*	VM_init		(char filename[]);
void		VM_shutdown	(RiscyVM* vm);
bool		VM_is_running	(RiscyVM* vm);

uint16_t	fetch		(RiscyVM* vm);
void		decode		(RiscyVM* vm, uint16_t instruction);
void		execute		();


#endif


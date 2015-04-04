#ifndef VM_H
#define VM_H

#include <stdbool.h>
#include <stdint.h>

typedef struct	RiscyVM		RiscyVM;

RiscyVM*	VM_init		(char filename[]);
void		VM_shutdown	(RiscyVM* vm);
bool		VM_is_running	(RiscyVM* vm);
void		VM_print_regs	(RiscyVM* vm);
void		VM_print_data	(RiscyVM* vm);

void		VM_fetch	(RiscyVM* vm);
void		VM_decode	(RiscyVM* vm);
void		VM_execute	(RiscyVM* vm);

#endif


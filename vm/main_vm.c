#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

/* TODO(Alexander):
 * 	- Read the TODOs in the vm.c file.
 * 	- For instructions, create an instruction struct to represent them.
 * 	  This struct should contain opcode, reg_ and _simm (see vm.c: decode)
 * 	- The fetch, decode and execute instruction will probably need new
 * 	  parameters and return values (see above point).
 */

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Usage: cpu <input_filename>\n");
		exit(EXIT_FAILURE);
	}

	char*	thisname	= argv[0];	/* Name of this C program */
	char*	progname	= argv[1];	/* Name of binary code file */

	RiscyVM* vm = VM_init(progname);

	while (VM_is_running(vm)) {
		uint16_t instruction = fetch(vm);
		decode(vm, instruction);
		execute();

		// Way cooler way to call the functions; need to change
		// parameters though
//		execute(vm, decode(vm, fetch(vm)));

//		memory_access();	// TODO: Remove this?
//		write_back();		// TODO: Remove this?
		printf("\n");
	}

	VM_shutdown(vm);

	printf("Program exited successfully.\n");
	return EXIT_SUCCESS;
}


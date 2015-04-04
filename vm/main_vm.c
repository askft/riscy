#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

/* TODO(Alexander)
 * 	- Look at all TODOs and XXXs in the vm.c file. They basically say:
 * 		* Make the file shorter?
 * 		* Does the program need to keep track of the instruction format?
 */

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("Usage: cpu <input_filename>\n");
		exit(EXIT_FAILURE);
	}

	char*	thisname	= argv[0];	/* Name of this C program */
	char*	progname	= argv[1];	/* Name of binary code file */
	bool	step		= false;

	if (argv[2] != NULL) {
		if (strcmp(argv[2], "--step") == 0) {
			step = true;
		} else {
			printf("[!] Usage: cpu <input_filename> [options]\n");
			exit(EXIT_FAILURE);
		}
	}

	RiscyVM* vm = VM_init(progname);	/* Start the virtual machine */

	while (VM_is_running(vm)) {

		VM_fetch(vm);
		VM_decode(vm);
		VM_execute(vm);

		if (step) {
			VM_print_regs(vm);
			VM_print_data(vm);

			printf("[ENTER]");
			getchar();
			printf("\n");
		}
	}

	if (!step) {
		VM_print_regs(vm);
		VM_print_data(vm);
	}

	VM_shutdown(vm);

	printf("Program exited successfully.\n");
	return EXIT_SUCCESS;
}


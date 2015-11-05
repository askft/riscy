#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

#define VERSION		"0.9.1"
#define WELCOME		"\n~~~~~ RiscyVM ~~~~~\n~~~~~ v."VERSION" ~~~~~\n\n"
#define EXIT_MESSAGE	"Program exited successfully.\n"

bool print_verbose_output;	/* Variables that are set */
bool step_through_program;	/* from program arguments */

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("Usage: run <input_filename>\n");
		exit(EXIT_FAILURE);
	}

	char* progname = argv[1];	/* Name of input file */

	for (int i = 2; i < argc; ++i) {
		if (!strcmp(argv[i], "--step"))
			step_through_program = true;
		else if (!strcmp(argv[i], "--verbose"))
			print_verbose_output = true;
		else {
			printf("Error: Unknown option \"%s\". Available "
				"options are:\n"
				"    --step      Step through the program.\n"
				"    --verbose   Print more information.\n",
				argv[i]);
			exit(EXIT_FAILURE);
		}
	}

	printf(WELCOME);

	/* Start the virtual machine */
	RiscyVM* vm = VM_init(progname);

	while (VM_is_running(vm)) {

		VM_fetch(vm);
		VM_decode(vm);
		VM_execute(vm);

		if (step_through_program) {
			VM_print_regs(vm);
			VM_print_data(vm);

			printf("[Press ENTER]");
			getchar();
			printf("\n");
		}
	}

	if (!step_through_program) {
		VM_print_regs(vm);
		VM_print_data(vm);
	}

	VM_shutdown(vm);
	vm = NULL;

	printf(EXIT_MESSAGE);
	return EXIT_SUCCESS;
}


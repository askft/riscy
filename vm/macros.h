#ifndef MACROS_H
#define MACROS_H



/*
 * If DEBUG != 0, activate debug mode (i.e. print more stuff)
 */
#define DEBUG	1



/* Format for printing. Only one of each should be active at a time.
 * 	- HEX	prints as "0x1234"
 * 	- DECS	prints as "   xyz"	where xyz is a signed 16-bit integer
 * 	- DECU	prints as "   xyz"	where xyz is an unsigned 16-bit integer
 */
#define PRINT_HEX		0
#define PRINT_DECS		1
#define PRINT_DECU		0
#define TABLE_PRINT_HEX		0
#define TABLE_PRINT_DECS	1
#define TABLE_PRINT_DECU	0



/* Error and debug messages */
#define OUT_OF_MEMORY	"Out of memory.\n"



/*
 * Function-like macros
 * 	ERROR:		Print location of error and exit with failure status.
 * 	DEBUG_VAR:	Print variable name and value.
 */

#define ERROR(...)							    \
do {									    \
	fprintf(stderr, "Error in file \"%s\", line %d, function \"%s\":\n",\
			__FILE__, __LINE__, __func__);			    \
	fprintf(stderr, __VA_ARGS__);					    \
	exit(EXIT_FAILURE);						    \
} while (0);

#if DEBUG
#define DEBUG_VAR(pre, x, post, var_format)			\
do {								\
	fprintf(stderr, pre "%s\t= "var_format post, #x, x);	\
} while (0);
#else
#define DEBUG_VAR(pre, x, post, var_format)
#endif

#if DEBUG
#define DEBUG_PRINT(...)					\
do {								\
	fprintf(stderr, "%s line %d: ", __FILE__, __LINE__);	\
	fprintf(stderr, __VA_ARGS__);				\
} while (0);
#else
#define DEBUG_PRINT(...)
#endif



/* Colors for terminal output.
 * Do not modify.
 */
#define KNRM	"\x1B[0m"
#define KRED	"\x1B[31m"
#define KGRN	"\x1B[32m"
#define KYEL	"\x1B[33m"
#define KBLU	"\x1B[34m"
#define KMAG	"\x1B[35m"
#define KCYN	"\x1B[36m"
#define KWHT	"\x1B[37m"
#define RESET	"\033[0m"



#if PRINT_HEX
	#define	PRINT_FORMAT	"0x%04x"
#elif PRINT_DECS
	#define PRINT_FORMAT	"%"PRId16
#elif PRINT_DECU
	#define PRINT_FORMAT	"%"PRIu16
#else
	#define PRINT_FORMAT	<<ERROR: PRINT_FORMAT is undefined>>
#endif

#if TABLE_PRINT_HEX
	#define TABLE_PRINT_FORMAT	"0x%04x"
#elif TABLE_PRINT_DECS
	#define TABLE_PRINT_FORMAT	"%6"PRId16
#elif TABLE_PRINT_DECU
	#define TABLE_PRINT_FORMAT	"%6"PRIu16
#else
	#define TABLE_PRINT_FORMAT <<ERROR: TABLE_PRINT_FORMAT is undefined>>
#endif


#endif /* MACROS_H */


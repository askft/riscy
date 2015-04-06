#ifndef PRINT_FORMAT_H
#define PRINT_FORMAT_H


/* Colors for terminal output. Do not modify. */
#define KNRM	"\x1B[0m"
#define KRED	"\x1B[31m"
#define KGRN	"\x1B[32m"
#define KYEL	"\x1B[33m"
#define KBLU	"\x1B[34m"
#define KMAG	"\x1B[35m"
#define KCYN	"\x1B[36m"
#define KWHT	"\x1B[37m"
#define RESET	"\033[0m"


/* Format for printing. Only one of each should be active at a time.
 * 	- HEX	prints as "0x1234"
 * 	- DECS	prints as "   xyz"	where xyz is a signed 16-bit integer
 * 	- DECU	prints as "   xyz"	where xyz is an unsigned 16-bit integer
 */
#define PRINT_HEX		0
#define PRINT_DECS		0
#define PRINT_DECU		1

#define TABLE_PRINT_DECU	0
#define TABLE_PRINT_DECS	0
#define TABLE_PRINT_HEX		1

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


#endif /* TPRINT_FORMAT_H */


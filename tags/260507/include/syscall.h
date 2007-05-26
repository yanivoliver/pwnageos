/*
Syscall handler
Author: Shimi G
*/

#ifndef HEADER_PWNAGE_SYSCALL
#define HEADER_PWNAGE_SYSCALL

#define NUMBER_OF_SYSCALL_ENTRIES	(255) /* The max value of AL*/

typedef bool_t (*syscall_handler_t)(registers_t * registers, struct syscall_entry_rec * syscall_entry);

typedef struct syscall_entry_rec {
	uchar_t key;
	bool_t blocking;
	syscall_handler_t handler;
	syscall_handler_t recall_handler;
} syscall_entry_t;

/*
Function name	: init_syscall
Purpose			: Initialize syscall table
Parameters		: None
*/
bool_t init_syscall();

/*
Function name	: install_syscall_handler
Purpose			: Install a syscall handler
Parameters		: key - AL key
				  handler - Syscall handler
Returns			: TRUE - Succes, FALSE - Failure
*/
bool_t install_syscall_handler(uchar_t key, syscall_handler_t handler, bool_t blocking, syscall_handler_t recall_handler);

/*
Function name	: uninstall_syscall_handler
Purpose			: Uninstall syscall handler
Parameters		: key - AL key
Returns			: TRUE - Succes, FALSE - Failure
*/
bool_t uninstall_syscall_handler(uchar_t key);

#endif


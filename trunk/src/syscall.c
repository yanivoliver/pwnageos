/*
Syscall handler
Author: Shimi G
*/

#include "common.h"
#include "interrupts.h"
#include "schedule.h"

void syscall_handler(ushort_t interrupt_number, registers_t * registers)
{
	/* Declare variables */
	process_t * process = NULL;

	/* Get the current process */
	process = get_current_process();

	/* POC - Enter blocking mode for user input 
	   Future - Search for sys-call handler by the AL register */
	/* Set to blocking mode */
	process->blocking = TRUE;

	schedule(0, registers);
}

bool_t init_syscall()
{
	/* Install the syscall */
	install_interrupt_handler(0x21, syscall_handler);

	/* Set the interrupts as user mode */
	set_interrupt_dpl(0x21, USER_PRIVILEGE);

	/* Return success */
	return TRUE;
}

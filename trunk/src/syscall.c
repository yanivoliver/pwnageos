/*
Syscall handler
Author: Shimi G
*/

#include "common.h"
#include "interrupts.h"

/* Set the syscall table */
syscall_entry_t g_syscall_table[NUMBER_OF_SYSCALL_ENTRIES] = {0};

void syscall_handler(ushort_t interrupt_number, registers_t * registers)
{
	/* Declare variables */
	process_t * process = NULL;
	uchar_t key = 0;
	bool_t results = FALSE;
	bool_t reschedule = FALSE;

	/* Get the current process */
	process = get_current_process();

	/* Find out the key of the syscall */
	key = (registers->eax >> 8) & 0xFF;

	/* Check key bounds */
	if (key < NUMBER_OF_SYSCALL_ENTRIES && 0 != g_syscall_table[key].key) {
		/* Key is valid */
		results = g_syscall_table[key].handler(registers, &g_syscall_table[key]);
		if (TRUE == g_syscall_table[key].blocking && FALSE == results) {
			/* Failed to get results and its a blocking syscall */
			process->blocking = TRUE;
			process->blocking_syscall = &g_syscall_table[key];
			reschedule = TRUE;			
		}
	} else {
		printf("Unknown syscall\n");
	}

	/* Check if we need to rechedule */
	if (TRUE == reschedule) {
		schedule(0, registers);
	}
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

bool_t install_syscall_handler(uchar_t key, syscall_handler_t handler, bool_t blocking, syscall_handler_t recall_handler)
{
	/* Check references */
	if (NULL == handler) {
		return FALSE;
	}
	if (TRUE == blocking && NULL == recall_handler) {
		return FALSE;
	}

	/* Check key bounds */
	if (key >= NUMBER_OF_SYSCALL_ENTRIES) {
		return FALSE;
	}

	/* Check if entry is empty */
	if (0 != g_syscall_table[key].key) {
		return FALSE;
	}

	/* Set entry */
	g_syscall_table[key].key = key;
	g_syscall_table[key].handler = handler;
	g_syscall_table[key].blocking = blocking;
	g_syscall_table[key].recall_handler = recall_handler;

	/* Return success */
	return TRUE;
}

bool_t uninstall_syscall_handler(uchar_t key)
{
	/* Check key bounds */
	if (key >= NUMBER_OF_SYSCALL_ENTRIES) {
		return FALSE;
	}

	/* Check if entry is empty */
	if (0 == g_syscall_table[key].key) {
		return FALSE;
	}

	/* Clear the entry */
	memset(&g_syscall_table[key], '\0', sizeof(syscall_entry_t));

	/* Return success */
	return TRUE;
}
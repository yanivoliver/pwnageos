/*
Handles irq's
Author: Shimi G.
Schedule handling
*/
#include "common.h"
#include "memory.h"
#include "schedule.h"
#include "tss.h"

process_t * g_current_process = NULL;
process_t * g_head_process = NULL;
ulong_t g_current_process_index = 0;
ulong_t g_base_stack = 0x50000;
ulong_t g_process_id = 0;
process_t g_process_list[NUMBER_OF_PROCESSES] = {0};

ulong_t create_process(ulong_t entry_point);

extern int getch();

bool_t init_schedule()
{
	/* Declare variables */

	/* Clear process list */
	memset(&g_process_list, '\0', NUMBER_OF_PROCESSES*sizeof(process_t));

	/* Process 0 -  This process should be deprecated from been used.
					it is used only as a jump process to the real process list when
					first entering multi-threaded mode */
	g_process_list[0].next_process = &g_process_list[1];

	/* Process 1*/
	create_process(idle);

	/* Process 3 */
	create_process(idle_second);

	/* Set the current process to the empty first entry */
	g_current_process = &g_process_list[0];

	/* Return success */
	return TRUE;
}

process_t * get_current_process()
{
	/* Return current process */
	return g_current_process;
}

/* This function is an abstract,
   in the future it will be replaced with dynamic allocation */
void * allocate_process_memory()
{
	/* Declare variables */
	ulong_t i = 0;

	/* Loop process array and look for an empty value */
	for (i = 1; i < NUMBER_OF_PROCESSES; i++) {
		if (0 == g_process_list[i].process_id) {
			/* Process is free */
			return &g_process_list[i];
		}
	}

	/* If we got here we didnt find any process memory */
	return NULL;
}

/*
Function: create_process
Purpse: Create a running process
*/
ulong_t create_process(ulong_t entry_point)
{
	/* Declare variables */
	process_t * process = NULL;

	/* Allocate process memory */
	process = allocate_process_memory();
	if (NULL == process) {
		/* No memory found */
		return 0;
	}

	/* Clear process struct memory */
	memset(process, '\0', sizeof(process_t));

	/* Set new process id */
	g_process_id++;
	process->process_id = g_process_id;

	/* Set information */
	process->blocking = FALSE;
	process->registers.ds = USER_DS;
	process->registers.es = USER_DS;
	process->registers.fs = USER_DS;
	process->registers.ss_iret = USER_DS;
	process->registers.esp_iret = 0x50000 + (0x1000 * process->process_id);
	process->registers.cs_iret = USER_CS;
	process->registers.eip_iret = entry_point;
	process->registers.eflags_iret = 0x0202;

	/* Connect the process */
	process->next_process = g_head_process;
	g_head_process = process;

	/* Return process id */
	return process->process_id;
}

void schedule(ushort_t irq, registers_t * registers)
{
	/* Declare variables */
	process_t * process = NULL;
	bool_t process_found = FALSE;
	uchar_t input = 0;

	/* Save all registers to the current process */
	g_current_process->registers.eax = registers->eax;
	g_current_process->registers.ecx = registers->ecx;
	g_current_process->registers.edx = registers->edx;
	g_current_process->registers.ebx = registers->ebx;
	g_current_process->registers.esp = registers->esp;
	g_current_process->registers.ebp = registers->ebp;
	g_current_process->registers.esi = registers->esi;
	g_current_process->registers.edi = registers->edi;
	g_current_process->registers.ds = registers->ds;
	g_current_process->registers.es = registers->es;
	g_current_process->registers.fs = registers->fs;

	g_current_process->registers.eip_iret = registers->eip_iret;
	g_current_process->registers.cs_iret = registers->cs_iret;
	g_current_process->registers.eflags_iret = registers->eflags_iret;
	g_current_process->registers.esp_iret = registers->esp_iret;
	g_current_process->registers.ss_iret = registers->ss_iret;

	while (TRUE != process_found) {
		/* Find a new process */
		process = g_current_process->next_process;

		/* Check that its not the end of the list */
		if (NULL == process) {
			process = g_head_process;
		}

		/* Set the new process */
		g_current_process = process;

		/* Check blocking mode */
		/* TODO: Call the helper function who blocked this thread, so it could be un-locked */
		if (TRUE == g_current_process->blocking) {
			/* Try to get character */
			input = get_char_from_queue();
			if (0 == input) {
				/* Stay in blocking mode, look for a new thread */
				process_found = FALSE;
			} else {
				/* Leave blocking mode, stay with the thread */
				g_current_process->blocking = FALSE;
				process_found = TRUE;

				/* Set the return value inside AL */
				g_current_process->registers.eax = input;
			}
		} else {
			/* We have found the process */
			process_found = TRUE;
		}
	}

	/* Load new process's registers */
	registers->eip_iret = g_current_process->registers.eip_iret;
	registers->eax = g_current_process->registers.eax;
	registers->ecx = g_current_process->registers.ecx;
	registers->edx = g_current_process->registers.edx;
	registers->ebx = g_current_process->registers.ebx;
	registers->esp = g_current_process->registers.esp;
	registers->ebp = g_current_process->registers.ebp;
	registers->esi = g_current_process->registers.esi;
	registers->edi = g_current_process->registers.edi;
	registers->ds = g_current_process->registers.ds;
	registers->es = g_current_process->registers.es;
	registers->fs = g_current_process->registers.fs;
	registers->eip_iret = g_current_process->registers.eip_iret;
	registers->cs_iret = g_current_process->registers.cs_iret;
	//registers->eflags_iret = g_current_process->registers.eflags_iret;
	registers->eflags_iret = 0x0202;
	registers->esp_iret = g_current_process->registers.esp_iret;
	registers->ss_iret = g_current_process->registers.ss_iret;


	//set_tss_available();
	/* From now on... the new process supposed to be loaded */
}

void idle()
{
	/* Do really nothing */
	for(;;) {
	}	
}

void idle_second()
{
	ulong_t i = 0;
	uchar_t input = 0;
	for(;;) {
		i++;
		printf("%c", getch());
	}	
}

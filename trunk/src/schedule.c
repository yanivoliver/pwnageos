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

	/* Process 1*/
	create_process(idle);

	/* Process 3 */
	create_process(idle_second);

	/* Process 1*/
	create_process(idle);

	/* Process 3 */
	create_process(idle_second);

	/* Set the current process to the empty first entry */
	g_current_process = &g_process_list[0];

	/* Return success */
	return TRUE;
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

	/* Find a new process */
	process = g_current_process->next_process;

	/* Check that its not the end of the list */
	if (NULL == process) {
		process = g_head_process;
	}

	/* Set the new process */
	g_current_process = process;

	//if (&g_process_list[1] == g_current_process) {
		/* Select the second process */
	//	g_current_process = &g_process_list[2];
	//} else {
		/* Select the first process */
	//	g_current_process = &g_process_list[1];
	//}

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
	ulong_t i = 0;
	for(;;) {
		i++;
		if (0 == i % 99999) {
			printf("A");
		}
	}	
}

void user_mode()
{
	for(;;) {
		__asm__("movl 0x2345, %eax");
	}	
}

void idle_second()
{
	ulong_t i = 0;
	for(;;) {
		i++;
		if (0 == i % 99999) {
			printf("B");
		}
	}	
}

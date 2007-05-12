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
process_t g_process_list[NUMBER_OF_PROCESSES] = {0};

bool_t init_schedule()
{
	/* Declare variables */

	/* Clear process list */
	memset(&g_process_list, '\0', NUMBER_OF_PROCESSES*sizeof(process_t));

	/* Add idle process */
	/* Entry zero is kept unused directly */

	/* Process 1*/
	g_process_list[1].process_id = 1;
	g_process_list[1].registers.ds = USER_DS | 0x0003;
	g_process_list[1].registers.es = USER_DS | 0x0003;
	g_process_list[1].registers.fs = USER_DS | 0x0003;
	g_process_list[1].registers.ss_iret = USER_DS | 0x0003;
	g_process_list[1].registers.esp_iret = 0x50000;
	g_process_list[1].registers.cs_iret = USER_CS | 0x0003;
	g_process_list[1].registers.eip_iret = idle;
	g_process_list[1].registers.eflags_iret = 0x0202;

	/* Process 2 */
	g_process_list[2].process_id = 1;
	g_process_list[2].registers.ds = USER_DS | 0x0003;
	g_process_list[2].registers.es = USER_DS | 0x0003;
	g_process_list[2].registers.fs = USER_DS | 0x0003;
	g_process_list[2].registers.ss_iret = USER_DS | 0x0003;
	g_process_list[2].registers.esp_iret = 0x80000;
	g_process_list[2].registers.cs_iret = USER_CS | 0x0003;
	g_process_list[2].registers.eip_iret = idle_second;
	g_process_list[2].registers.eflags_iret = 0x0202;

	/* Set the current process to the empty first entry */
	g_current_process = &g_process_list[0];

	/* Return success */
	return TRUE;
}

void schedule(ushort_t irq, registers_t * registers)
{
	/* Declare variables */

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
	if (&g_process_list[1] == g_current_process) {
		/* Select the second process */
		g_current_process = &g_process_list[2];
	} else {
		/* Select the first process */
		g_current_process = &g_process_list[1];
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
		printf("%d,", i);
		i++;
		//__asm__("cli");
		//printf("!");
		//print_tss();
		//__asm__("sti");
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
	
	
	for(;;) {
		//__asm__("cli");
		printf(".");
		//__asm__("sti");
	}
	
}

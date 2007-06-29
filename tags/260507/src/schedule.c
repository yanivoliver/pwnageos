/*
Handles irq's
Author: Shimi G.
Schedule handling
*/
#include "common.h"
#include "memory.h"
#include "string.h"
#include "process.h"
#include "schedule.h"
#include "syscall.h"
#include "keyboard.h"
#include "tss.h"

process_t * g_current_process = NULL;
process_t * g_head_process = NULL;
ulong_t g_current_process_index = 0;
ulong_t g_base_stack = 0x50000;
ulong_t g_process_id = 0;
process_t g_process_list[NUMBER_OF_PROCESSES] = {0};

extern int getch();
extern int getchar();
extern int putch(int c);
extern int gets(char * buffer);
extern void puts(char * buffer);

ulong_t init_schedule()
{
	/* Declare variables */
	ulong_t idle_process_id = 0;

	/* Clear process list */
	memset(g_process_list, '\0', NUMBER_OF_PROCESSES*sizeof(process_t));

	/* Process 0 -  This process should be deprecated from been used.
					it is used only as a jump process to the real process list when
					first entering multi-threaded mode */
	g_process_list[0].next_process = &g_process_list[1];

	/* Process 1*/
	idle_process_id = create_process(idle, "System");

	/* Process 2 */
	create_process(idle_fourth, "Testing simple input");
	
	/* Process 3 */
	create_process(idle_second, "Testing interactive input");

	/* Process 4 */
	create_process(idle_third, "Testing output");

	/* Set the current process to the empty first entry */
	g_current_process = NULL;

	/* Return success */
	return idle_process_id;
}

process_t * get_current_process()
{
	/* Return current process */
	return g_current_process;
}

process_t * get_head_process()
{
	/* Return head process */
	return g_head_process;
}

process_t * get_last_process()
{
	/* Declare variables */
	ulong_t i = 0;
	process_t * process = NULL;

	/* Set head process */
	process = get_head_process();

	/* Iterate all processes */
	while (NULL != process->next_process) {
		/* Get next process */
		process = process->next_process;
	}

	/* Return the process */
	return process;
}

process_t * get_process_by_console(console_t * console)
{
	/* Declare variables */
	ulong_t i = 0;
	process_t * process = NULL;
	
	/* Check references */
	if (NULL == console) {
		return NULL;
	}

	/* Set head process */
	process = get_head_process();

	/* Iterate all processes */
	do {
		/* Check screen */
		if (console == &process->console) {
			/* Found console */
			return process;
		}

		/* Set next process */
		process = process->next_process;		
	} while (NULL != process);

	/* Process not found */
	return NULL;
}

process_t * get_process_by_id(ulong_t process_id)
{
	/* Declare variables */
	ulong_t i = 0;
	process_t * process = NULL;

	/* Set head process */
	process = get_head_process();

	/* Iterate all processes */
	do {
		/* Check screen */
		if (process_id == process->process_id) {
			/* Found console */
			return process;
		}

		/* Set next process */
		process = process->next_process;		
	} while (NULL != process);

	/* Process not found */
	return NULL;
}

process_t * get_prev_process(process_t * process_seek)
{
	/* Declare variables */
	ulong_t i = 0;
	process_t * process = NULL;

	/* Check references */
	if (NULL == process_seek) {
		return NULL;
	}

	/* Set head process */
	process = get_head_process();

	/* Check head */
	if (process_seek == process->next_process) {
		return process;
	}

	/* Iterate all processes */
	while (NULL != process->next_process) {
		/* Check the process */
		if (process_seek == process->next_process) {
			return process;
		}

		/* Get next process */
		process = process->next_process;
	}

	/* Return the process */
	return NULL;
}

/* This function is an abstract,
   in the future it will be replaced with dynamic allocation */
static void * allocate_process_memory()
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
static ulong_t create_process(ulong_t entry_point, uchar_t * name)
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

	/* Set the input information */
	process->input.current_key = KEYQUEUE_NEW_QUEUE;
	process->input.empty_node = 0;
	memset(&process->input.key_buffer, '\0', sizeof(keyboard_queue_entry) * KEY_QUEUE_SIZE);

	/* Set information */
	process->blocking = FALSE;
	process->blocking_syscall = NULL;
	process->registers.ds = USER_DS;
	process->registers.es = USER_DS;
	process->registers.fs = USER_DS;
	process->registers.ss_iret = USER_DS;
	process->registers.esp_iret = 0x90000 + (0x1000 * process->process_id);
	process->registers.cs_iret = USER_CS;
	process->registers.eip_iret = entry_point;
	process->registers.eflags_iret = 0x0202;

	if (NULL != name) {
		/* Copy name */
		memcpy(process->name, name, STRING_BUFFER-1);
		process->name[STRING_BUFFER-1] = '\0';
	} else {
		process->name[0] = '\0';
	}

	/* Set console */
	process->console.row = 1;
	process->console.row_header = 1;
	process->console.column = 0;
	process->console.color = SCREEN_DEFAULT_COLOR;
	process->console.foreground = SCREEN_FOREGROUND_COLOR;
	process->console.background = SCREEN_BACKGROUND_COLOR;

	clrscr(&process->console);
	draw_header(&process->console, process->name);

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
	ulong_t i = 0;

	if (NULL == g_current_process) {
		g_current_process = g_head_process;
	} else {
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
	}

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
		if (TRUE == g_current_process->blocking) {
			/* Call the recall handler of the syscall */
			if (TRUE != g_current_process->blocking_syscall->recall_handler(&g_current_process->registers, g_current_process->blocking_syscall)) {
				/* Syscall is not ready */
				process_found = FALSE;
			} else {
				/* Syscall is done */
				process_found = TRUE;
				g_current_process->blocking = FALSE;
				g_current_process->blocking_syscall = 0;
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
	char buffer[100] = {0};
	char ch = 0;
	bool_t valid_choice = FALSE;

	for(;;) {
		puts("===== IO Tests =====\n");
		puts("     1) Print hello\n");
		puts("     2) Get character input\n");
		puts("     3) Get string input\n");

		do {
			puts("Please choose one: ");
			ch = getchar();
			puts("\n");

			if (ch == '1' || ch == '2' || ch == '3') {
				valid_choice = TRUE;
			} else {
				valid_choice = FALSE;
			}
		} while (FALSE == valid_choice);

		puts("\n===== IO Results =====\n");
		switch (ch) {
		case '1':
			puts("Hello world !\n");
			break;
		case '2':
			puts("Enter character: ");
			ch = getchar();
			puts("\nYou entered ");
			putch(ch);
			putch('\n');
			break;
		case '3':
			puts("Enter you name: ");
			gets(buffer);
			puts("\nYour name is ");
			puts(buffer);
			putch('\n');
			break;
		}
		putch('\n');
	}	
}

void idle_third()
{
	ulong_t i = 0;
	for(;;) {
		i++;
		if (0 == i % 99999) {
			puts("Writing,");
		}
	}	
}

void idle_fourth()
{
	char buffer[1024] = {0};
	for(;;) {
		gets(buffer);
		puts(" / ");
		puts(buffer);
		putch('\n');
	}	
}
/*
Handles irq's
Author: Shimi G.
Schedule handling
*/
#include "common.h"
#include "memory.h"
#include "irq.h"
#include "string.h"
#include "process.h"
#include "schedule.h"
#include "syscall.h"
#include "keyboard.h"
#include "tss.h"
#include "gdt.h"

#define STACK_DONT_FIX				(0)
#define STACK_FIX_KERNEL_TO_USER	(1)
#define STACK_FIX_USER_TO_KERNEL	(2)

process_t * g_current_process = NULL;
process_t * g_head_process = NULL;
ulong_t g_current_process_index = 0;
ulong_t g_base_stack = 0x50000;
ulong_t g_process_id = 0;
process_t g_process_list[NUMBER_OF_PROCESSES] = {0};
ulong_t g_scheduler_fixed_stack = STACK_DONT_FIX;
bool_t g_scheduling_enabled = FALSE;

extern void load_ldtr(ushort_t * row);
extern int getch();
extern int getchar();
extern int putch(int c);
extern int gets(char * buffer);
extern void puts(char * buffer);
extern void fread(void * buffer, ulong_t length);

extern void enable_interrupts();
extern void disable_interrupts();


void empty()
{
	for (;;) {
		printf(NULL, "A");
	}
}

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

	/* Set initial status of stack fixing as false */
	g_scheduler_fixed_stack = FALSE;

	/* Process 1*/
	idle_process_id = create_process(idle, "System");

	/* Process 2 */
	///create_process(idle/*_fourth*/, "Testing simple input");
	
	/* Process 3 */
	///create_process(idle/*_second*/, "Testing interactive input");

	/* Process 4 */
	///create_process(idle/*_third*/, "Testing output");

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

bool_t is_scheduling_enabled()
{
	return g_scheduling_enabled;
}

/*
Function: create_process
Purpse: Create a running process
*/
static ulong_t create_process(ulong_t entry_point, uchar_t * name)
{
	/* Declare variables */
	process_t * process = NULL;
	ulong_t tss_entry_index = 0;

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

	/* Create and set tss information */
	tss_entry_index = create_tss();
	if (0 == tss_entry_index) {
		return 0;
	}
	process->tss_entry_index = tss_entry_index;

	/* Set the input information */
	process->input.current_key = KEYQUEUE_NEW_QUEUE;
	process->input.empty_node = 0;
	memset(&process->input.key_buffer, '\0', sizeof(keyboard_queue_entry) * KEY_QUEUE_SIZE);

	/* Set information */
	process->blocking = FALSE;
	process->blocking_syscall = NULL;
	process->kernel_mode = FALSE;
	process->registers.ds = USER_DS;
	process->registers.es = USER_DS;
	process->registers.fs = USER_DS;
	process->registers.ss_iret = USER_DS;
	process->registers.esp_iret = 0x90000 + (0x1000 * process->process_id);
	process->registers.cs_iret = USER_CS;
	process->registers.eip_iret = entry_point;
	process->registers.eflags_iret = 0x0202;

	process->kernel_stack = 0x400000 + (0x1000 * process->process_id);

	process->registers_kernel.ds = KERNEL_DS;
	process->registers_kernel.es = KERNEL_DS;
	process->registers_kernel.fs = KERNEL_DS;
	process->registers_kernel.ss_iret = KERNEL_DS;
	process->registers_kernel.esp = process->kernel_stack;
	process->registers_kernel.cs_iret = KERNEL_CS;
	process->registers_kernel.eip_iret = entry_point;
	process->registers_kernel.eflags_iret = 0x0202;

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
	registers_t * registers_pointer = NULL;
	registers_t * registers_fixed = NULL;
	tss_t * tss = NULL;
	bool_t process_found = FALSE;
	uchar_t input = 0;
	ulong_t i = 0;
}

void schedule_old(ushort_t irq, registers_t * registers)
{
	/* Declare variables */
	process_t * process = NULL;
	registers_t * registers_pointer = NULL;
	registers_t * registers_fixed = NULL;
	tss_t * tss = NULL;
	bool_t process_found = FALSE;
	uchar_t input = 0;
	ulong_t i = 0;

	g_scheduling_enabled = TRUE;

	/* If we interruped another irq we dont need to reschedule now */
	/* TODO: Reschedule flag after the interrupted irq finished */
	if (TRUE == is_dispatching_irq()) {
		//tss = get_tss();
		//tss->ss_0 = KERNEL_DS;
		//tss->esp_0 = g_current_process->registers_kernel.esp;
		//printf(NULL, "[PROC]");
		return;
	}

	if (NULL == g_current_process) {
		g_current_process = g_head_process;
	} else {
		/* Save all registers to the current process */
		/* Check if we are saving kernel or user registers */
		if (USER_CS == registers->cs_iret) {
			registers_pointer = &g_current_process->registers;

			/* We freezed the process in USER mode */
			g_current_process->kernel_mode = FALSE;
		} else {
			registers_pointer = &g_current_process->registers_kernel;

			/* We freezed the process in KERNEL mode */
			g_current_process->kernel_mode = TRUE;
		}
		registers_pointer->eax = registers->eax;
		registers_pointer->ecx = registers->ecx;
		registers_pointer->edx = registers->edx;
		registers_pointer->ebx = registers->ebx;
		registers_pointer->esp = registers->esp;
		registers_pointer->ebp = registers->ebp;
		registers_pointer->esi = registers->esi;
		registers_pointer->edi = registers->edi;
		registers_pointer->ds = registers->ds;
		registers_pointer->es = registers->es;
		registers_pointer->fs = registers->fs;

		/* If its kernel mode we need to fix the esp */
		if (TRUE == g_current_process->kernel_mode) {
			registers_pointer->esp += 0x14;
		} else {
			registers_pointer->esp += 0x28;
		}

		registers_pointer->eip_iret = registers->eip_iret;
		registers_pointer->cs_iret = registers->cs_iret;
		registers_pointer->eflags_iret = registers->eflags_iret;

		/* This values actualy exist only in the transition from kernel to user */
		/* They are used in kernel registers to hold pointer to the stack */
		if (FALSE == g_current_process->kernel_mode) {
			registers_pointer->esp_iret = registers->esp_iret;
			registers_pointer->ss_iret = registers->ss_iret;
		}
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

	/* If we are going to return to a user mode thread when the interrupted thread is a kernel one
	   we should fixup the stack and add 2 parameters of the ss and esp for the ring 0 to ring 3 change */
	if (FALSE == g_current_process->kernel_mode && KERNEL_CS == registers->cs_iret) {
		registers_fixed = (registers_t *)(((ulong_t)registers)-0x8);
		g_scheduler_fixed_stack = STACK_FIX_KERNEL_TO_USER;
	} else if (TRUE == g_current_process->kernel_mode && USER_CS == registers->cs_iret) {
		registers_fixed = (registers_t *)(((ulong_t)registers)+0x8);
		g_scheduler_fixed_stack = STACK_FIX_USER_TO_KERNEL;
	} else {
		registers_fixed = registers;
	}

	/* Check if we are loading kernel or user registers */
	if (FALSE == g_current_process->kernel_mode) {
		registers_pointer = &g_current_process->registers;
	} else {
		registers_pointer = &g_current_process->registers_kernel;
	}

	/* Load new process's registers */
	registers_fixed->eip_iret = registers_pointer->eip_iret;
	registers_fixed->eax = registers_pointer->eax;
	registers_fixed->ecx = registers_pointer->ecx;
	registers_fixed->edx = registers_pointer->edx;
	registers_fixed->ebx = registers_pointer->ebx;
	registers_fixed->esp = registers_pointer->esp;
	registers_fixed->ebp = registers_pointer->ebp;
	registers_fixed->esi = registers_pointer->esi;
	registers_fixed->edi = registers_pointer->edi;
	registers_fixed->ds = registers_pointer->ds;
	registers_fixed->es = registers_pointer->es;
	registers_fixed->fs = registers_pointer->fs;
	registers_fixed->eip_iret = registers_pointer->eip_iret;
	registers_fixed->cs_iret = registers_pointer->cs_iret;
	registers_fixed->eflags_iret = g_current_process->registers.eflags_iret;
	registers_fixed->eflags_iret |= 0x202;

	/* We change this values only when leaving to lower ring */
	if (FALSE == g_current_process->kernel_mode) {
		registers_fixed->esp_iret = g_current_process->registers.esp_iret;
		registers_fixed->ss_iret = g_current_process->registers.ss_iret;
		
	}

	/* Change the TSS values to the current's process kernel stack */
	//tss = get_tss();

	/* Change tss values */
	////tss->ss_0 = KERNEL_DS;
	////tss->esp_0 = g_current_process->kernel_stack;
	g_current_process->kernel_mode = FALSE;

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
	
	//fread(buffer, 80);

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
	uchar_t i = 'A';
	for(;;) {
		i++;
		putch('A');
		if ('J' == i) {
			i = 'A'-1;
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

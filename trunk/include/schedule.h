/*
Handles irq's
Author: Shimi G.
Schedule handling
*/

#ifndef HEADER_PWNAGE_SCHEDULE
#define HEADER_PWNAGE_SCHEDULE

#include "syscall.h"
#include "screen.h"

#define NUMBER_OF_PROCESSES	(10)

/* Process*/
typedef struct process_rec {
	ulong_t process_id;
	registers_t registers;
	bool_t blocking;
	syscall_entry_t * blocking_syscall;
	console_t console;
	struct process_rec * next_process;
} process_t;

/*
Function name	: init_schedule
Purpose			: Initialize scheduler
Parameters		: None
*/
bool_t init_schedule();

/*
Function name	: schedule
Purpose			: Schedule tasks
Parameters		: irq - The irq of the timer
				  registers - The registeres of the previous thread
*/
void schedule(ushort_t irq, registers_t * registers);

/*
Function name	: get_current_process
Purpose			: Get current process
Parameters		: None
*/
process_t * get_current_process();

/*
Function name	: get_head_process
Purpose			: Get head process
Parameters		: None
*/
process_t * get_head_process();

/*
Function name	: get_last_process
Purpose			: Get last process
Parameters		: None
*/
process_t * get_last_process();

/*
Function name	: get_process_by_console
Purpose			: Find a process by console value
Parameters		: console - The console to look for
Returns			: Success - Process, Failure - NULL
*/
process_t * get_process_by_console(console_t * console);

/*
Function name	: get_process_by_id
Purpose			: Find a process by its id
Parameters		: process_id - The process id to look for
Returns			: Success - Process, Failure - NULL
*/
process_t * get_process_by_id(ulong_t process_id);

/*
Function name	: get_prev_process
Purpose			: Finds the previous process in the list
Parameters		: process - The process to look for
Returns			: Success - Process, Failure - NULL
*/
process_t * get_prev_process(process_t * process_seek);

/*
Function name	: idle
Purpose			: To do nothing :)
Parameters		: None
*/
void idle();

/*
Function name	: idle_second
Purpose			: To do even more nothing :)
Parameters		: None
*/
void idle_second();


#endif

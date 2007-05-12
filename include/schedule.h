/*
Handles irq's
Author: Shimi G.
Schedule handling
*/

#ifndef HEADER_PWNAGE_SCHEDULE
#define HEADER_PWNAGE_SCHEDULE

#define NUMBER_OF_PROCESSES	(10)

/* Registers */
typedef struct registers_rec {
	ulong_t fs;
	ulong_t es;
	ulong_t ds;
	ulong_t edi;
	ulong_t esi;
	ulong_t ebp;
	ulong_t esp;
	ulong_t ebx;
	ulong_t edx;
	ulong_t ecx;
	ulong_t eax;
	ulong_t interrupt_number;
	ulong_t error_code;
	ulong_t eip_iret;
	ulong_t cs_iret;
	ulong_t eflags_iret;
	ulong_t esp_iret;
	ulong_t ss_iret;
} registers_t;

/* Process*/
typedef struct process_rec {
	ulong_t process_id;
	registers_t registers;
	struct process_rec * next_process;
	/*
	TODO - List of threads
	*/
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

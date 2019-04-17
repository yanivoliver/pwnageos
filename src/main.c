/*
System loader
Author: Shimi G.
*/
#include "common.h"
#include "interrupts.h"
#include "schedule.h"
#include "irq.h"
#include "io.h"
#include "screen.h"
#include "keyboard.h"
#include "syscall.h"
#include "gdt.h"
#include "dma.h"
#include "tss.h"
#include "floppy.h"

/* Extern from kernel_low_level */
extern void infinite_loop();
void empty_irq_handler(ushort_t irq, registers_t * registers);

/*#define MAIN_INITIALIZE(DEVICE_NAME, DEVICE_DESCRIPTION)	\
	printf(NULL, "INIT"DEVICE_DESCRIPTION);	\
	if (TRUE == init_##DEVICE_NAME()) {						\
		printf(NULL, "D\n");							\
	} else {												\
		printf(NULL, "E\n");							\
		infinite_loop();									\
	}*/


#define MAIN_INITIALIZE(DEVICE_NAME, DEVICE_DESCRIPTION)	\
	printf(NULL, "Initializing "DEVICE_DESCRIPTION" ... ");	\
	if (TRUE == init_##DEVICE_NAME()) {						\
		console_foreground(NULL, SCREEN_COLOR_RED);			\
		printf(NULL, "[Done]\n");							\
		console_foreground(NULL, SCREEN_COLOR_WHITE);		\
	} else {												\
		console_foreground(NULL, SCREEN_COLOR_CYAN);		\
		printf(NULL, "[Error]\n");							\
		console_foreground(NULL, SCREEN_COLOR_WHITE);		\
		infinite_loop();									\
	}

int main(void)
{
	/* Declare variables */
	ulong_t idle_process_id = 0;

	/* Initialize screen and scheduler without any messages */
	if (TRUE != init_gdt()) {
		/* Error initializing gdt */
		infinite_loop();
	}

	if (TRUE != init_tss()) {
		/* Error initializing tss */
		infinite_loop();
	}

	idle_process_id = init_schedule();
	if (0 == idle_process_id) {
		/* Error initializing scheduler */
		infinite_loop();
	}

	if (TRUE != init_screen(idle_process_id)) {
		/* Error initializing screen */
		infinite_loop();
	}

	MAIN_INITIALIZE(interrupts, "interrupts");
	MAIN_INITIALIZE(gdt, "gdt table");
	MAIN_INITIALIZE(tss, "tss");
	MAIN_INITIALIZE(syscall, "syscall manager");
	MAIN_INITIALIZE(keyboard, "keyboard");
	MAIN_INITIALIZE(dma, "dma");
	MAIN_INITIALIZE(floppy, "floppy");

	install_irq_handler(0, schedule);
	enable_irq(0);

	//tss_jump();

	/* Enter into an infinite loop */
	infinite_loop();

	/* Return which doesnt ever supposed to happend*/
	return 0;
}


void empty_irq_handler(ushort_t irq, registers_t * registers)
{
	printf(NULL," HAHA");
}
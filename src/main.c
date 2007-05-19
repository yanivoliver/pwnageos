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
#include "tss.h"

/* Extern from kernel_low_level */
extern void infinite_loop();

int main(void)
{
	/* Declare variables */
	ulong_t idle_process_id = 0;

	/* Initialize screen and scheduler without any messages */
	idle_process_id = init_schedule();
	if (0 == idle_process_id) {
		/* Error initializing scheduler */
		infinite_loop();
	}

	if (TRUE != init_screen(idle_process_id)) {
		/* Error initializing screen */
		infinite_loop();
	}

	/* Clear screen */
	//clrscr();

	/* Print pre-init message */
	printf("Initializing interrupts ... ");

	/* Initialize interrupts */
	if (TRUE == init_interrupts()) {
		/* Print post-init message*/
		console_foreground(0x4);
		printf("[Done]\n");
		console_foreground(0xF);
	} else {
		/* Error */
		console_foreground(0x3);
		printf("[Error]\n");
		console_foreground(0xF);

		/* Enter panic mode */
		infinite_loop();
	}

	/* Print pre-init message */
	printf("Initializing gdt table ... ");

	/* Initialize interrupts */
	if (TRUE == init_gdt()) {
		/* Print post-init message*/
		console_foreground(0x4);
		printf("[Done]\n");
		console_foreground(0xF);
	} else {
		/* Error */
		console_foreground(0x3);
		printf("[Error]\n");
		console_foreground(0xF);

		/* Enter panic mode */
		infinite_loop();
	}

	/* Print pre-init message */
	printf("Initializing tss ... ");

	/* Initialize interrupts */
	if (TRUE == init_tss()) {
		/* Print post-init message*/
		console_foreground(0x4);
		printf("[Done]\n");
		console_foreground(0xF);
	} else {
		/* Error */
		console_foreground(0x3);
		printf("[Error]\n");
		console_foreground(0xF);

		/* Enter panic mode */
		infinite_loop();
	}

	/* Print pre-init message */
	printf("Initializing syscall manager ... ");

	/* Initialize interrupts */
	if (TRUE == init_syscall()) {
		/* Print post-init message*/
		console_foreground(0x4);
		printf("[Done]\n");
		console_foreground(0xF);
	} else {
		/* Error */
		console_foreground(0x3);
		printf("[Error]\n");
		console_foreground(0xF);

		/* Enter panic mode */
		infinite_loop();
	}

	/* Print pre-init message */
	printf("Initializing keyboard ... ");

	/* Initialize interrupts */
	if (TRUE == init_keyboard()) {
		/* Print post-init message*/
		console_foreground(0x4);
		printf("[Done]\n");
		console_foreground(0xF);
	} else {
		/* Error */
		console_foreground(0x3);
		printf("[Error]\n");
		console_foreground(0xF);

		/* Enter panic mode */
		infinite_loop();
	}
	
	__asm__("int $0x80");
	__asm__("int $0x44");
	__asm__("int $0x64");
	__asm__("int $0x3");
	__asm__("int $0x7F");

	/* Re-install irq handler of the timer */
	install_irq_handler(0, schedule);

	/* Enter into an infinite loop */
	infinite_loop();

	/* Return which doesnt ever supposed to happend*/
	return 0;
}

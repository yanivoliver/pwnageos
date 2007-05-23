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
void empty_irq_handler(ushort_t irq, registers_t * registers);

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
	printf(NULL, "Initializing interrupts ... ");

	/* Initialize interrupts */
	if (TRUE == init_interrupts()) {
		/* Print post-init message*/
		console_foreground(NULL, 0x4);
		printf(NULL, "[Done]\n");
		console_foreground(NULL, 0xF);
	} else {
		/* Error */
		console_foreground(NULL, 0x3);
		printf(NULL, "[Error]\n");
		console_foreground(NULL, 0xF);

		/* Enter panic mode */
		infinite_loop();
	}

	/* Print pre-init message */
	printf(NULL, "Initializing gdt table ... ");

	/* Initialize interrupts */
	if (TRUE == init_gdt()) {
		/* Print post-init message*/
		console_foreground(NULL, 0x4);
		printf(NULL, "[Done]\n");
		console_foreground(NULL, 0xF);
	} else {
		/* Error */
		console_foreground(NULL, 0x3);
		printf(NULL, "[Error]\n");
		console_foreground(NULL, 0xF);

		/* Enter panic mode */
		infinite_loop();
	}

	/* Print pre-init message */
	printf(NULL, "Initializing tss ... ");

	/* Initialize interrupts */
	if (TRUE == init_tss()) {
		/* Print post-init message*/
		console_foreground(NULL, 0x4);
		printf(NULL, "[Done]\n");
		console_foreground(NULL, 0xF);
	} else {
		/* Error */
		console_foreground(NULL, 0x3);
		printf(NULL, "[Error]\n");
		console_foreground(NULL, 0xF);

		/* Enter panic mode */
		infinite_loop();
	}

	/* Print pre-init message */
	printf(NULL, "Initializing syscall manager ... ");

	/* Initialize interrupts */
	if (TRUE == init_syscall()) {
		/* Print post-init message*/
		console_foreground(NULL, 0x4);
		printf(NULL, "[Done]\n");
		console_foreground(NULL, 0xF);
	} else {
		/* Error */
		console_foreground(NULL, 0x3);
		printf(NULL, "[Error]\n");
		console_foreground(NULL, 0xF);

		/* Enter panic mode */
		infinite_loop();
	}

	/* Print pre-init message */
	printf(NULL, "Initializing keyboard ... ");

	/* Initialize interrupts */
	if (TRUE == init_keyboard()) {
		/* Print post-init message*/
		console_foreground(NULL, 0x4);
		printf(NULL, "[Done]\n");
		console_foreground(NULL, 0xF);
	} else {
		/* Error */
		console_foreground(NULL, 0x3);
		printf(NULL, "[Error]\n");
		console_foreground(NULL, 0xF);

		/* Enter panic mode */
		infinite_loop();
	}
	
	__asm__("int $0x80");
	__asm__("int $0x7F");

	/* Re-install irq handler of the timer */
	install_irq_handler(8, empty_irq_handler);
	enable_irq(8);

	install_irq_handler(0, schedule);
	enable_irq(0);

	/* Enter into an infinite loop */
	infinite_loop();

	/* Return which doesnt ever supposed to happend*/
	return 0;
}


void empty_irq_handler(ushort_t irq, registers_t * registers)
{
	printf(NULL, "7,");
}
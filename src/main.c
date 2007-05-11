/*
System loader
Author: Shimi G.
*/
#include "common.h"
#include "interrupts.h"
#include "irq.h"
#include "io.h"
#include "screen.h"
#include "gdt.h"
#include "tss.h"

/* Extern from kernel_low_level */
extern void infinite_loop();

int main(void)
{
	/* Clear screen */
	clrscr();

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

	__asm__("int $0x80");
	__asm__("int $0x44");
	__asm__("int $0x64");
	__asm__("int $0x3");
	__asm__("int $0xFF");
	__asm__("int $0x7F");

	/* Enter into an infinite loop */
	infinite_loop();

	/* Return which doesnt ever supposed to happend*/
	return 0;
}

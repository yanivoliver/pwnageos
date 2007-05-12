/*
Handles irq's
Author: Shimi G.
IRQ Handler - Inspired by GeekOS
*/
#include "common.h"
#include "io.h"
#include "interrupts.h"
#include "schedule.h"
#include "irq.h"
#include "tss.h"

/* Global variable for the irq mask */
ushort_t g_irq_mask = 0xfffb;

/* Global table of irq handlers */
irq_handler_t g_irq_handlers[NUMBER_OF_IRQ_ENTRIES] = {0};

/* Where the irq are mapped in the interrupt table */
extern uchar_t g_first_external_interrupt;
extern void common_irq_handler(ushort_t interrupt_number, registers_t * registers);

/* Macros to handle irq mask which is WORD and we use only one byte at a time */
#define MASTER(mask)	((mask) & 0xff)
#define SLAVE(mask)		(((mask)>>8) & 0xff)

bool_t install_irq_handler(ushort_t irq, irq_handler_t handler)
{
	/* Check the irq number */
	if (IRQ_HIGH_LIMIT < irq || IRQ_LOW_LIMIT > irq) {
		/* Corrupt irq number, return failure */
		return FALSE;
	}

	/* Install an interrupt handler with an offset to the first external interrupt */
	/* PIC's are moved to be in a sequence */
	if (TRUE == install_interrupt_handler(irq + g_first_external_interrupt, common_irq_handler)) {
		/* Set irq handler */
		g_irq_handlers[irq] = handler;

		/* Success installing the interrupt handler */
		return TRUE;
	} else {
		/* Error installing interrupt handler */
		return FALSE;
	}
}

bool_t uninstall_irq_handler(ushort_t irq)
{
	/* Check the irq number */
	if (IRQ_HIGH_LIMIT < irq || IRQ_LOW_LIMIT > irq) {
		/* Corrupt irq number, return failure */
		return FALSE;
	}

	/* When an irq is uninstalled we explicitly disable it */
	disable_irq(irq);

	/* Remove irq handler */
	g_irq_handlers[irq] = NULL;

	/* Uninstall an interrupt handler with an offset to the first external interrupt */
	/* PIC's are moved to be in a sequence */
    return uninstall_interrupt_handler(irq + g_first_external_interrupt);
}

/*
Function name	: get_irq_mask
Purpose			: Get the current irq mask
Parameters		: None
*/
ushort_t get_irq_mask()
{
    return g_irq_mask;
}

/*
Function name	: set_irq_mask
Purpose			: Set a new irq mask
Parameters		: mask - The new irq mask
*/
void set_irq_mask(ushort_t mask)
{
	/* Variables*/
    uchar_t mask_old;
	uchar_t mask_new;

	/* Get the old mask */
    mask_old = MASTER(g_irq_mask);

	/* Set the new mask */
    mask_new = MASTER(mask);

	/* Check if there were any change */
    if (mask_new != mask_old) {
		/* There was a change, actualy send the change to the controller */
		out(PIC_0_DATA, mask_new);
    }

	/* Get the old mask */
    mask_old = SLAVE(g_irq_mask);

	/* Set the new mask */
    mask_new = SLAVE(mask);

	/* Check if there were any change */
    if (mask_new != mask_old) {
		/* There was a change, actualy send the change to the controller */
		out(PIC_1_DATA, mask_new);
    }

	/* Set the global value */
    g_irq_mask = mask;
}

void enable_irq(ushort_t irq)
{
	/* Variables */
    ushort_t mask = 0;

	/* Get the global mask */
	mask = get_irq_mask();

	/* Set the new mask value */
    mask &= ~(1 << irq);

	/* Actually send the new value*/
    set_irq_mask(mask);
}

void disable_irq(ushort_t irq)
{
	/* Variables */
    ushort_t mask = 0;

	/* Get the global mask */
	mask = get_irq_mask();

	/* Set the new mask value */
    mask |= (1 << irq);

	/* Actually send the new value*/
    set_irq_mask(mask);
}

void common_irq_handler(ushort_t interrupt_number, registers_t * registers)
{
	/* Variables */
	uchar_t command = 0;
	ushort_t irq = 0;

	/* Calculate the irq number */
	irq = interrupt_number - g_first_external_interrupt;

	/* Find and appropriate irq handler */
	if (NULL != g_irq_handlers[irq]) {
		/* Call the irq handler */
		g_irq_handlers[irq](irq, registers);
	} else {
		/* No default handler found. Print an appropriate message */
		printf("Irq 0x%X called, no deafult handler found !\n", irq);
	}

	/* End the irq session with the PIC chip */
	/* Calculate the command to the PIC handler */
	command = 0x60 | (irq & 0x7);

    if (8 > irq) {
		/* Send a message only the the first PIC */
		out(PIC_0_CONTROL, command);
    } else {
		/* Send a message to each PIC*/
		out(PIC_1_CONTROL, command);
		out(PIC_0_CONTROL, 0x62);
    }
}

/*
Function name	: begin_irq
Purpose			: To be called at a begining of an irq
Parameters		: irq - The irq to be disabled
*/
/*void begin_irq(ushort_t irq)
{
}*/

/*
 * Called by an IRQ handler to end the interrupt.
 * Sends an EOI command to the appropriate PIC(s).
 */
/*void end_irq(ushort_t irq)
{
    //ushort_t irq = state->intNum - g_first_external_interrupt;
    uchar_t command = 0x60 | (irq & 0x7);

    if (irq < 8) {
		out(0x20, command);
    } else {
		out(0xA0, command);
		out(0x20, 0x62);
    }
}*/

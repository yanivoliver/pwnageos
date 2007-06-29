/*
Handles irq's
Author: Shimi G.
This file is taken as is from GeekOS and altered to fit the project
*/

#ifndef HEADER_PWNAGE_IRQ
#define HEADER_PWNAGE_IRQ

#include "schedule.h"

/* Set where the PIC's are located */
#define PIC_0_CONTROL			(0x20)
#define PIC_0_DATA				(0x21)
#define PIC_1_CONTROL			(0xA0)
#define PIC_1_DATA				(0xA1)

#define NUMBER_OF_IRQ_ENTRIES	(16)
#define IRQ_HIGH_LIMIT			(15)
#define IRQ_LOW_LIMIT			(0)


/* Irq handler */
typedef void (*irq_handler_t)(ushort_t irq, registers_t * registers);

/*
Function name	: install_irq_handler
Purpose			: Install an irq
Parameters		: irq - The irq to install the handler to
				  handler - And interrupt handler
*/
bool_t install_irq_handler(ushort_t irq, irq_handler_t handler);

/*
Function name	: uninstall_irq_handler
Purpose			: Uninstall an irq
Parameters		: irq - The irq to uninstall
Side effects	: Irq is explicitly disabled using disable_irq() function
*/
bool_t uninstall_irq_handler(ushort_t irq);

/*
Function name	: enable_irq
Purpose			: Enable an irq
Parameters		: irq - The irq to be enabled
*/
void enable_irq(ushort_t irq);

/*
Function name	: disable_irq
Purpose			: Disable an irq
Parameters		: irq - The irq to be disabled
*/
void disable_irq(ushort_t irq);

/*
Function name	: common_irq_handler
Purpose			: Common handler for all irq calls. Used as a gateway to the irq handler from the idt handler
Parameters		: interrupt_number - The number of the interrupt
				  registers - Thread registers
*/
void common_irq_handler(ushort_t interrupt_number, registers_t * registers);

/*
Function name	: is_irq_interrupt
Purpose			: Checks whether an interrupt number is an irq or not
Parameters		: interrupt_number - The number of the interrupt
				  registers - Thread registers
*/
bool_t is_irq_interrupt(ulong_t interrupt_number);

/*
Function name	: is_dispatching_irq
Purpose			: Check wether an irq is dispatching
Parameters		: None
Returns			: TRUE - IRQ is dispatching
				  FALSE - IRQ is not dispatching
*/
bool_t is_dispatching_irq();

/*
Function name	: set_irq_dispatching
Purpose			: Set the irq dispatching status
Parameters		: dispatch - Is irq dispatching
Returns			: void
*/
void set_irq_dispatching(bool_t dispatch);

#endif

/*
Interrupts header
Author: Shimi G.
*/

#ifndef HEADER_PWNAGE_INTERRUPTS
#define HEADER_PWNAGE_INTERRUPTS

#include "schedule.h"

/* Number of IDT entries */
#define NUMBER_OF_IDT_ENTRIES		(256)
#define IDT_ENTRY_SIZE				(8)
#define INTERRUPT_GATE_SIGNATURE	(0x70)
#define TRAP_GATE_SIGNATURE			(0x78)
#define INTERRUPT_HIGH_LIMIT		(256)
#define INTERRUPT_LOW_LIMIT			(0)

/* Entry for an IDT interrupt */
typedef struct idt_interrupt_entry_rec {
	ushort_t offset_low;
	ushort_t segment_selector;
	unsigned reserved : 5;
	unsigned signature : 8;
	unsigned dpl : 2;
	unsigned present : 1;
	ushort_t offset_high;
} idt_interrupt_entry_t;

/* Interrupt handler */
typedef void (*interrupt_handler_t)(ushort_t interrupt_number, registers_t * registers);

/*
Function name	: init_interrupts
Purpose			: Initialize interrupts
Parameters		: None
*/
bool_t init_interrupts();

/*
Function name	: init_idt
Purpose			: Initialize IDT table and load it
Parameters		: None
*/
void init_idt();

/*
Function name	: init_interrupt_gate
Purpose			: Initialize and idt entry as an interrupt gate
Parameters		: entry - IDT entry
				  dpl - access level
				  address - address of the handler
*/
void init_interrupt_gate(idt_interrupt_entry_t * entry, ushort_t dpl, ulong_t address);
void init_trap_gate(idt_interrupt_entry_t * entry, ushort_t dpl, ulong_t address);

/*
Function name	: install_interrupt_handler
Purpose			: Install a high-level handler to an interrupt
Parameters		: interrupt_number - The interrupt number
				  handler - Function pointer of an handler
Return			: TRUE - Success
				  FALSE - Failure
*/
bool_t install_interrupt_handler(ushort_t interrupt_number, interrupt_handler_t handler);

/*
Function name	: uninstall_interrupt_handler
Purpose			: Uninstall a high-level handler from an interrupt
Parameters		: interrupt_number - The interrupt number
Return			: TRUE - Success
				  FALSE - Failure
*/
bool_t uninstall_interrupt_handler(ushort_t interrupt_number);

/* Temp function for debug */
void sys_call_handler(ushort_t interrupt_number, registers_t * registers);
void keyboard_handler_2(ushort_t irq, registers_t * registers);
void gpf_handler(ushort_t interrupt_number, registers_t * registers);

#endif

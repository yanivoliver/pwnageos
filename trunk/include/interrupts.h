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
#define EFLAGS_IF					(0x200)

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
Function name	: set_interrupt_dpl
Purpose			: Sets the dpl of an interrupt
Parameters		: interrupt_number - The interrupt number
				  dpl - Function pointer of an handler
Return			: TRUE - Success
				  FALSE - Failure
*/
bool_t set_interrupt_dpl(ushort_t interrupt_number, ushort_t dpl);

/*
Function name	: uninstall_interrupt_handler
Purpose			: Uninstall a high-level handler from an interrupt
Parameters		: interrupt_number - The interrupt number
Return			: TRUE - Success
				  FALSE - Failure
*/
bool_t uninstall_interrupt_handler(ushort_t interrupt_number);

/*
Function name	: is_interrupts_enabled
Purpose			: Check if interrupts are enabled or not
Parameters		: None
Return			: TRUE - Interrupts are enabled
				  FALSE - Interrupts are disabled
*/
bool_t is_interrupts_enabled();

/*
Function name	: atomic_disable_interrupts
Purpose			: Disable interrupts only if they are not disabled, it will return the previous state
				  which could be recovered using atomic_enable_interrupts
Parameters		: None
Return			: TRUE - Interrupts are enabled
				  FALSE - Interrupts are disabled
*/
bool_t atomic_disable_interrupts();
void atomic_enable_interrupts(bool_t restore_interrupts);

/* Temp function for debug */
void gpf_handler(ushort_t interrupt_number, registers_t * registers);

#endif

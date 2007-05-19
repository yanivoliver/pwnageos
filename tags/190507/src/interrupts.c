/*
Handles interrupts
Author: Shimi G.
*/
#include "common.h"
#include "interrupts.h"
#include "schedule.h"
#include "tss.h"

extern void configure_pic();
extern void load_ldtr(ushort_t * row);
extern void enable_interrupts();
extern void disable_interrupts();

extern void g_basic_interrupt_handlers_table();
extern ulong_t g_basic_interrupt_handler_size;

idt_interrupt_entry_t	g_idt_table[NUMBER_OF_IDT_ENTRIES]		= {0};
interrupt_handler_t		g_idt_handlers[NUMBER_OF_IDT_ENTRIES]	= {0};

bool_t init_interrupts()
{
	/* Initialize and load IDT table*/
	configure_pic();
	init_idt();
	enable_interrupts();

	/* Return success */
	return TRUE;
}

void empty_handler()
{
}

void init_idt()
{
	/* Variables */
	ushort_t i = 0;
	ushort_t idt_pointer[3];

	ulong_t handler_table = (ulong_t)g_basic_interrupt_handlers_table;
	ulong_t handler_entry = handler_table;
	const ulong_t handler_size = g_basic_interrupt_handler_size;

	/* Loop all idt entries */
	for (i = 0; i < NUMBER_OF_IDT_ENTRIES; i++) {
		/* Initialize as interrupt gate */
		init_trap_gate(&g_idt_table[i], KERNEL_PRIVILEGE, handler_entry);

		/* Move to the next entry in the handler table */
		handler_entry += handler_size;
	}

	/* Install sys-call handler */
	install_interrupt_handler(0x0D, gpf_handler);

	/* Install timer handler */
	install_irq_handler(0, empty_handler);
	enable_irq(0);

	/* Set the idt register value */
	idt_pointer[0] = IDT_ENTRY_SIZE * NUMBER_OF_IDT_ENTRIES;
    idt_pointer[1] = (ulong_t) g_idt_table & 0xffff;
    idt_pointer[2] = (ulong_t) g_idt_table >> 16;

	/* Load the idt table */
	load_idtr(idt_pointer);
}

void init_interrupt_gate(idt_interrupt_entry_t * entry, ushort_t dpl, ulong_t address)
{
	/* Check references */
	if (NULL == entry) {
		/* Bad references */
		return;
	}

	/* Check DPL */
	if (dpl != USER_PRIVILEGE && dpl != KERNEL_PRIVILEGE) {
		return;
	}

	/* Set the entry */
    entry->offset_low = address & 0xffff;
    entry->segment_selector = KERNEL_CS;
    entry->reserved = 0;
    entry->signature = INTERRUPT_GATE_SIGNATURE;
    entry->dpl = dpl;
    entry->present = 1;
    entry->offset_high = address >> 16;
}

void init_trap_gate(idt_interrupt_entry_t * entry, ushort_t dpl, ulong_t address)
{
	/* Check references */
	if (NULL == entry) {
		/* Bad references */
		return;
	}

	/* Check DPL */
	if (dpl != USER_PRIVILEGE && dpl != KERNEL_PRIVILEGE) {
		return;
	}

	/* Set the entry */
    entry->offset_low = address & 0xffff;
    entry->segment_selector = KERNEL_CS;
    entry->reserved = 0;
    entry->signature = TRAP_GATE_SIGNATURE;
    entry->dpl = dpl;
    entry->present = 1;
    entry->offset_high = address >> 16;
}

bool_t install_interrupt_handler(ushort_t interrupt_number, interrupt_handler_t handler)
{
	/* Check the interrupt number */
	if (INTERRUPT_HIGH_LIMIT < interrupt_number || INTERRUPT_LOW_LIMIT > interrupt_number) {
		/* Corrupt interrupt number, return failure */
		return FALSE;
	}

	/* Set the handler */
	g_idt_handlers[interrupt_number] = handler;

	/* Return success */
	return TRUE;
}

bool_t set_interrupt_dpl(ushort_t interrupt_number, ushort_t dpl)
{
	/* Check the interrupt number */
	if (INTERRUPT_HIGH_LIMIT < interrupt_number || INTERRUPT_LOW_LIMIT > interrupt_number) {
		/* Corrupt interrupt number, return failure */
		return FALSE;
	}

	/* Check DPL */
	if (dpl != USER_PRIVILEGE && dpl != KERNEL_PRIVILEGE) {
		return FALSE;
	}

	/* Set the handler */
	g_idt_table[interrupt_number].dpl = dpl;

	/* Return success */
	return TRUE;
}

bool_t uninstall_interrupt_handler(ushort_t interrupt_number)
{
	/* Check the interrupt number */
	if (INTERRUPT_HIGH_LIMIT < interrupt_number || INTERRUPT_LOW_LIMIT > interrupt_number) {
		/* Corrupt interrupt number, return failure */
		return FALSE;
	}

	/* Set the handler */
	g_idt_handlers[interrupt_number] = NULL;

	/* Return success */
	return TRUE;
}

void gpf_handler(ushort_t interrupt_number, registers_t * registers)
{
	printf("** General protection fault\n");
}

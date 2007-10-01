/*
Handles interrupts
Author: Shimi G.
*/
#include "common.h"
#include "interrupts.h"
#include "irq.h"
#include "schedule.h"
#include "tss.h"

extern void configure_pic();
extern void load_ldtr(ushort_t * row);
extern void load_tr(ushort_t row);
extern void scheduler_low();

extern void enable_interrupts();
extern void disable_interrupts();
extern ulong_t get_eflags();

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
	for (;;) {
	}
}

void init_idt()
{
	/* Variables */
	ushort_t i = 0;
	ushort_t idt_pointer[3];
	ulong_t scheduler_tss_index = 0;
	tss_t * scheduler_tss = NULL;

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
	//install_interrupt_handler(0x0D, gpf_handler);

	/*scheduler_tss_index = create_tss();
	scheduler_tss = get_tss(scheduler_tss_index);
	scheduler_tss->es = KERNEL_DS;
	scheduler_tss->cs = KERNEL_CS;
	scheduler_tss->ss = KERNEL_DS;
	scheduler_tss->ds = KERNEL_DS;
	scheduler_tss->fs = KERNEL_DS;
	scheduler_tss->gs = KERNEL_DS;
	scheduler_tss->es = KERNEL_DS;
	scheduler_tss->ss_0 = KERNEL_DS;
	scheduler_tss->esp_0 = 0;
	scheduler_tss->eip = 0;
	scheduler_tss->io_map = 0xFFFF;
	load_tr(segment_selector(KERNEL_PRIVILEGE, TRUE, scheduler_tss_index));
	set_tss_available(scheduler_tss_index);*/

	/* Install timer handler */
	scheduler_tss_index = create_tss();
	scheduler_tss = get_tss(scheduler_tss_index);
	scheduler_tss->es = KERNEL_DS;
	scheduler_tss->cs = KERNEL_CS;
	scheduler_tss->ss = KERNEL_DS;
	scheduler_tss->ds = KERNEL_DS;
	scheduler_tss->fs = KERNEL_DS;
	scheduler_tss->gs = KERNEL_DS;
	scheduler_tss->es = KERNEL_DS;
	scheduler_tss->ss_0 = KERNEL_DS;
	scheduler_tss->esp_0 = 0x300000;
	scheduler_tss->esp = 0x300000;
	scheduler_tss->eip = scheduler_low;
	scheduler_tss->io_map = 0xFFFF;
	//printf(NULL, "[%d]", scheduler_tss_index);
	//init_call_gate(&g_idt_table[0x30], KERNEL_PRIVILEGE, scheduler_tss_index);
	//install_call_gate(0x30, KERNEL_PRIVILEGE, scheduler_tss_index);
	init_interrupt_gate(&g_idt_table[0x30], USER_PRIVILEGE, scheduler_low);
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

void init_call_gate(idt_interrupt_entry_t * entry, ushort_t dpl, ulong_t tss_entry_index)
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
	entry->offset_low = 0;
    entry->segment_selector = segment_selector(dpl, TRUE, tss_entry_index);
    entry->reserved = 0;
    entry->signature = CALL_GATE_SIGNATURE;
    entry->dpl = dpl;
    entry->present = 1;
	entry->offset_high = 0;
}

bool_t install_call_gate(ushort_t interrupt_number, ushort_t dpl, ulong_t tss_entry_index)
{
	/* Declare variables */
	ushort_t segment_sel = 0;

	/* Check the interrupt number */
	if (INTERRUPT_HIGH_LIMIT < interrupt_number || INTERRUPT_LOW_LIMIT > interrupt_number) {
		/* Corrupt interrupt number, return failure */
		return FALSE;
	}

	/* Check DPL */
	if (dpl != USER_PRIVILEGE && dpl != KERNEL_PRIVILEGE) {
		return;
	}

	/* Get the segment selector */
	segment_sel = segment_selector(KERNEL_PRIVILEGE, TRUE, tss_entry_index);

	/* Set the entry */
    g_idt_table[interrupt_number].segment_selector = segment_sel;
    g_idt_table[interrupt_number].reserved = 0;
    g_idt_table[interrupt_number].signature = CALL_GATE_SIGNATURE;
    g_idt_table[interrupt_number].dpl = dpl;
    g_idt_table[interrupt_number].present = 1;
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

bool_t is_interrupts_enabled()
{
	ulong_t eflags = 0;	
	eflags = get_eflags();
	if (0 == EFLAGS_IF & eflags) {
		return FALSE;
	} else {
		return TRUE;
	}
}

bool_t atomic_disable_interrupts()
{
	bool_t interrupts_enabled = is_interrupts_enabled();
	if (TRUE == interrupts_enabled && TRUE == is_scheduling_enabled())	{
		disable_interrupts();
	}

	return interrupts_enabled;
}

void atomic_enable_interrupts(bool_t restore_interrupts)
{
	if (TRUE == restore_interrupts && TRUE == is_scheduling_enabled()) {
		enable_interrupts();
	}
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
	//printf(NULL, "** General protection fault\n");
}

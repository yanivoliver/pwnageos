/*
Handles interrupts
Author: Shimi G.
*/
#include "common.h"
#include "interrupts.h"
#include "tss.h"

extern void configure_pic();
extern void load_ldtr(ushort_t * row);
extern void enable_interrupts();
extern void disable_interrupts();

extern void g_basic_interrupt_handlers_table();
extern ulong_t g_basic_interrupt_handler_size;

idt_interrupt_entry_t	g_idt_table[NUMBER_OF_IDT_ENTRIES]		= {0};
interrupt_handler_t		g_idt_handlers[NUMBER_OF_IDT_ENTRIES]	= {0};

char keyboard_map[]			= {	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
								'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
								'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0, 0, 0,
								'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'};

char keyboard_map_shift[]	= {	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
								'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0,
								'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 0, 0, 0,
								'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'};

#define SHIFT_LEFT			(0x2A)
#define SHIFT_RIGHT			(0x36)

#define CONTROL_SHIFT		(0x1)
#define CONTROL_CONTROL		(0x2)
#define CONTROL_ALT			(0x4)
#define CONTROL_CAPSLOCK	(0x8)
#define CONTROL_NUMLOCK		(0x10)

#define KEY_RELEASED(KEY)	((KEY) | 0x80)

uchar_t g_control_keys = 0;

bool_t init_interrupts()
{
	/* Initialize and load IDT table*/
	configure_pic();
	init_idt();
	enable_interrupts();

	/* Return success */
	return TRUE;
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
	install_interrupt_handler(0xFF, sys_call_handler);

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

void sys_call_handler(ushort_t interrupt_number, registers_t * registers)
{
	printf("SYS CALL !!!\n");
}

void timer_handler(ushort_t irq, registers_t * registers)
{
	/* Go into user-mode */
	ulong_t i = 0;
	//if (0xBABEBABE == registers->ebx)
	//{
		printf("[%X%X%X%X,", registers->eax);
		//printf("%X%X%X%X,", g_tss.esp_0);
		print_tss();
		printf("%X%X%X%X]", registers->edx);
	//}
}

void keyboard_handler(ushort_t irq, registers_t * registers)
{
	uchar_t scan_code = 0;

	/* Read the scan code */
	scan_code = in(0x60);

	/* Check if its a shift buttons */
	if (SHIFT_LEFT == scan_code || SHIFT_RIGHT == scan_code) {	
		/* Enable shift effect */
		g_control_keys |= CONTROL_SHIFT;

		/* We have no use of printing this character here */
		return;
	} else if (KEY_RELEASED(SHIFT_LEFT) == scan_code || KEY_RELEASED(SHIFT_RIGHT) == scan_code) {
		/* Disable shift effect */
		g_control_keys &= ~CONTROL_SHIFT;

		/* We have no use of printing this character here */
		return;
	}

	/* Ignore high-bitted codes */
	if (0 != (scan_code & 0x80)) {
		return;
	}

	/* Print the value to the screen */
	if (0 != (g_control_keys & CONTROL_SHIFT)) {
		printf("%c", keyboard_map_shift[scan_code]);
	} else {
		printf("%c", keyboard_map[scan_code]);
	}
}


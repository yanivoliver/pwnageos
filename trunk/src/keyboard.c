/*
Keyboard handler
Author: Yaniv Oliver (and some Shimi :P)
*/

#include "common.h"
#include "schedule.h"
#include "keyboard.h"
#include "memory.h"
#include "screen.h"
#include "interrupts.h"
#include "syscall.h"
#include "irq.h"

#define KEY_RELEASED(KEY)			((KEY) | 0x80)
#define KEY_QUEUE_SIZE				(10) /* Defines the size of the key queue */
#define KEYQUEUE_NEW_QUEUE			(-1)
#define KEYQUEUE_NO_EMPTY_NODES		(-2)
#define CONSOLE_BACKWARD_TRIGGER	(59)
#define CONSOLE_FOREWARD_TRIGGER	(60)

/* Holds a pressed key entry. */
typedef struct
{
	uchar_t keycode;
	uchar_t control_keys;
} keyboard_queue_entry;

uchar_t g_keyboard_map[]			= {	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
								'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
								'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0, 0, 0,
								'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '};

uchar_t g_keyboard_map_shift[]	= {	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
								'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0,
								'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 0, 0, 0,
								'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '};


/* Create the key queue */
keyboard_queue_entry key_buffer[KEY_QUEUE_SIZE] = {{0, 0}};

/* Points to the current key in queue */
int g_current_key = KEYQUEUE_NEW_QUEUE;
int g_empty_node = 0;

void insert_keycode_to_queue(uchar_t scan_code, uchar_t control_keys)
{
	/* Check if there are no empty nodes */
	if (KEYQUEUE_NO_EMPTY_NODES == g_empty_node) {
		return;
	}

	/* Insert the key pressed into an empty entry */
	key_buffer[g_empty_node].keycode = scan_code;
	key_buffer[g_empty_node].control_keys = control_keys;

	/* If the current key is -1, it means the queue is still empty, so make the current key point to us */
	g_current_key = g_empty_node;

	/* Check if the queue is full (we reached the end of the queue) */
	if (KEY_QUEUE_SIZE == g_empty_node) {
		/* If it isn't empty, the queue if full */
		g_empty_node = KEYQUEUE_NO_EMPTY_NODES;
	} else {
		/* And if the queue isn't full */
		/* Check if the next queue entry is empty and free to be used */
		if (0 == key_buffer[g_empty_node + 1].keycode) {
			g_empty_node++;
		} else {
			/* If it isn't empty, the queue is full */
			g_empty_node = KEYQUEUE_NO_EMPTY_NODES;
		}
	}
}

uchar_t get_char_from_queue()
{
	uchar_t return_char = 0;

	/* Check if its a new queue, if so, no keys are in the buffer yet. Also check if the current keycode
	   is empty, which means we don't want to go forward in the queue for the next call to the funciton. */
	if (KEYQUEUE_NEW_QUEUE == g_current_key) {
		return return_char;
	}

	/* Check if shift is on */
	if (CONTROL_SHIFT == CONTROL_SHIFT && key_buffer[g_current_key].control_keys) {
		/* Return the shifted char */
		return_char = g_keyboard_map_shift[key_buffer[g_current_key].keycode];
	} else {
		/* Otherwise */
		/* Return the non shifted char */
		return_char = g_keyboard_map[key_buffer[g_current_key].keycode];
	}

	/* Zero out the current queue entry */
	key_buffer[g_current_key].keycode = 0;
	key_buffer[g_current_key].control_keys = 0;

	/* Check if we are at the first character */
	if (0 == g_current_key) {
		/* Set the first character as the empty one */
		g_current_key = KEYQUEUE_NEW_QUEUE;
		g_empty_node = 0;
	} else {
		/* Set the empty node to us and set the new current key */
		g_current_key--;
		g_empty_node--;
	}
	
	return return_char;
}

void keyboard_handler(ushort_t irq, registers_t * registers)
{
	uchar_t scan_code = 0; /* Holds the scan code of the key pressed */
	static uchar_t control_keys = 0; /* Holds the control key for the current key pressed */

	/* Read the scan code */
	scan_code = in(0x60);

	/* Check if its a shift buttons */
	if (SHIFT_LEFT == scan_code || SHIFT_RIGHT == scan_code) {	
		/* Enable shift effect */
		control_keys |= CONTROL_SHIFT;

		/* We have no use of printing this character here */
		return;
	} else if (KEY_RELEASED(SHIFT_LEFT) == scan_code || KEY_RELEASED(SHIFT_RIGHT) == scan_code) {
		/* Disable shift effect */
		control_keys &= ~CONTROL_SHIFT;

		/* We have no use of printing this character here */
		return;
	}

	/* Ignore high-bitted codes (key released, other than control keys we don't really care for now) */
	if (0 != (scan_code & 0x80)) {
		return;
	}

	/* Check if we are trying to make a console switch */
	switch (scan_code) {
		case CONSOLE_BACKWARD_TRIGGER:
			show_prev_console();
			return;
			break;
		case CONSOLE_FOREWARD_TRIGGER:
			show_next_console();
			return;
			break;
	}

	/* Insert the key pressed into the queue */
	insert_keycode_to_queue(scan_code, control_keys);
}

bool_t syscall_char_read_recall(registers_t * registers, struct syscall_entry_rec * syscall_entry)
{
	/* Declare variables */
	uchar_t input = 0;

	/* Get character */
	input = get_char_from_queue();

	/* Check input */
	if (0 == input) {
		/* No valid input */
		return FALSE;
	} else {
		/* Found character, set the return value inside eax */
		registers->eax = input;

		/* Return success*/
		return TRUE;
	}
}

bool_t syscall_char_read(registers_t * registers, struct syscall_entry_rec * syscall_entry)
{
	/* Clear the input buffer */
	memset(key_buffer, '\0', KEY_QUEUE_SIZE * sizeof(keyboard_queue_entry));
	g_current_key = KEYQUEUE_NEW_QUEUE;
	g_empty_node = 0;

	/* Call the recall function */
	return syscall_entry->recall_handler(registers, syscall_entry);
}

bool_t syscall_char_write(registers_t * registers, struct syscall_entry_rec * syscall_entry)
{
	/* Declare variables */
	uchar_t output = 0;
	process_t * process = NULL;

	/* Get outut char from DL */
	output = registers->edx & 0xFF;

	/* Check if its a tab, and set the return value of the last printed character */
	if (0x09 == output) {
		registers->eax &= 0xFFFFFF00;
		registers->eax |= 0x00000020;
	} else {
		registers->eax &= 0xFFFFFF00;
		registers->eax |= output;
	}

	/* Print the character */
	process = get_current_process();
	set_working_console(&process->console);
	printf("%c", output);
	set_working_console(NULL);

	/* Return success */
	return TRUE;
}

bool_t init_keyboard()
{
	/* Install the keyboard */
	install_irq_handler(1, keyboard_handler);
	enable_irq(1);

	/* Make all entries empty */
	memset(key_buffer, '\0', sizeof(keyboard_queue_entry) * KEY_QUEUE_SIZE);

	/* Registers syscall */
	install_syscall_handler(0x08, syscall_char_read, TRUE, syscall_char_read_recall);
	install_syscall_handler(0x02, syscall_char_write, FALSE, NULL);

	return TRUE;
}
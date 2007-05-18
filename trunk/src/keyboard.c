/*
Keyboard handler
Author: Yaniv Oliver (and some Shimi :P)
*/

#include "common.h"
#include "schedule.h"
#include "keyboard.h"
#include "interrupts.h"
#include "irq.h"

#define KEY_RELEASED(KEY)			((KEY) | 0x80)
#define KEY_QUEUE_SIZE				(10) /* Defines the size of the key queue */
#define KEYQUEUE_NEW_QUEUE			(-1)
#define KEYQUEUE_NO_EMPTY_NODES		(-2)

/* Holds a pressed key entry. */
typedef struct
{
	uchar_t keycode;
	uchar_t control_keys;
} keyboard_queue_entry;

uchar_t g_keyboard_map[]			= {	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
								'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
								'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0, 0, 0,
								'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'};

uchar_t g_keyboard_map_shift[]	= {	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
								'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0,
								'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 0, 0, 0,
								'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'};


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
	if (KEYQUEUE_NEW_QUEUE == g_current_key) {
		g_current_key = g_empty_node;
	}

	/* Check if the queue is full (we reached the end of the queue) */
	if (KEY_QUEUE_SIZE == g_empty_node) {
		/* Check if the first position in the queue is empty */
		if (0 == key_buffer[0].keycode) {
			/* If it is, make the next empty node point there */
			g_empty_node = 0;
		} else {
			/* If it isn't empty, the queue if full */
			g_empty_node = KEYQUEUE_NO_EMPTY_NODES;
		}
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
	if (KEYQUEUE_NEW_QUEUE == g_current_key || 0 == key_buffer[g_current_key].keycode) {
		return 0;
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

	/* Check if the queue is currently full, so we can point the queue pointer to this new empty node to use */
	if (KEYQUEUE_NO_EMPTY_NODES == g_empty_node) {
		g_empty_node = g_current_key;
	}

	/* Check if this is the end of the queue */
	if (KEY_QUEUE_SIZE == g_current_key) {
		/* If it is, reset to the first queue entry */
		g_current_key = 0;
	} else {
		/* Otherwise increment the queue pointer */
		g_current_key++;
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

	/* Insert the key pressed into the queue */
	insert_keycode_to_queue(scan_code, control_keys);
}

bool_t init_keyboard()
{
	/* Install the keyboard */
	install_irq_handler(1, keyboard_handler);
	enable_irq(1);

	return TRUE;
}

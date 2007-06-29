/*
Keyboard include
Author: Yaniv Oliver
*/

#ifndef HEADER_PWNAGE_KEYBOARD
#define HEADER_PWNAGE_KEYBOARD

#define SHIFT_LEFT					(0x2A)
#define SHIFT_RIGHT					(0x36)

#define CONTROL_SHIFT				(0x1)
#define CONTROL_CONTROL				(0x2)
#define CONTROL_ALT					(0x4)
#define CONTROL_CAPSLOCK			(0x8)
#define CONTROL_NUMLOCK				(0x10)

#define KEY_RELEASED(KEY)			((KEY) | 0x80)
#define KEYQUEUE_NEW_QUEUE			(-1)
#define KEYQUEUE_NO_EMPTY_NODES		(-2)
#define CONSOLE_BACKWARD_TRIGGER	(59)
#define CONSOLE_FOREWARD_TRIGGER	(60)
#define CONSOLE_LIST_TRIGGER		(61)
#define KEY_QUEUE_SIZE				(64) /* Defines the size of the key queue */

/* Holds a pressed key entry. */
typedef struct
{
	uchar_t keycode;
	uchar_t control_keys;
} keyboard_queue_entry;

typedef struct input_rec {
	int current_key;
	int empty_node;
	keyboard_queue_entry key_buffer[KEY_QUEUE_SIZE];
} input_t;


/*
Function name	: init_keyboard
Purpose			: Initialize keyboard
Parameters		: None
*/
bool_t init_keyboard();

#endif


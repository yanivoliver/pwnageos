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

/*
Function name	: init_keyboard
Purpose			: Initialize keyboard
Parameters		: None
*/
bool_t init_keyboard();

#endif


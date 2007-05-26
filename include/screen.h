/*
Screen header
Author: Shimi G.
*/

#ifndef HEADER_PWNAGE_SCREEN
#define HEADER_PWNAGE_SCREEN

#define SCREEN_COLUMNS			(80)
#define SCREEN_ROWS				(25)

#define SCREEN_BACKGROUND_COLOR	(0x0)
#define SCREEN_FOREGROUND_COLOR	(0xF)

#define SCREEN_DEFAULT_COLOR	((SCREEN_BACKGROUND_COLOR << 4) | SCREEN_FOREGROUND_COLOR)

#define SCREEN_EMPTY_CHARACTER	(0x00)
#define CHARACTER_TERMINATOR	(0x00)

#define SCREEN_SEGMENT			(0xB8000)

typedef struct console_rec {
	uchar_t screen[SCREEN_ROWS*SCREEN_COLUMNS*2];
	ushort_t row;
	ushort_t row_header;
	ushort_t column;
	ushort_t color;
	ushort_t foreground;
	ushort_t background;
} console_t;

typedef enum screen_color_e {
	SCREEN_COLOR_BLACK = 0x0,
	SCREEN_COLOR_BLUE = 0x1,
	SCREEN_COLOR_GREEN = 0x2,
	SCREEN_COLOR_CYAN = 0x3,
	SCREEN_COLOR_RED = 0x4,
	SCREEN_COLOR_MAGENTA = 0x5,
	SCREEN_COLOR_BROWN = 0x6,
	SCREEN_COLOR_GRAY_LIGHT = 0x7,
	SCREEN_COLOR_GRAY_DARK = 0x8,
	SCREEN_COLOR_BLUE_LIGHT = 0x9,
	SCREEN_COLOR_GREEN_LIGHT = 0xA,
	SCREEN_COLOR_CYAN_LIGHT = 0xB,
	SCREEN_COLOR_RED_LIGHT = 0xC,
	SCREEN_COLOR_MAGENTA_LIGHT = 0xD,
	SCREEN_COLOR_YELLOW = 0xE,
	SCREEN_COLOR_WHITE = 0xF
} screen_color_t;


#include "process.h"

/*
Function name	: init_screen
Purpose			: Initialize screen
Parameters		: None
Returns			: TRUE - Success, FALSE - Failure
*/
bool_t init_screen();

/*
Function name	: set_working_console
Purpose			: Set working console which will receive all writings
Parameters		: console - The console
Returns			: Previous console
*/
//console_t * set_working_console(console_t * console);

/*
Function name	: show_next_console
Purpose			: Show the next console
Parameters		: None
Returns			: None
*/
void show_next_console();

/*
Function name	: show_prev_console
Purpose			: Show the next console
Parameters		: None
Returns			: None
*/
void show_prev_console();

/*
Function name	: show_console
Purpose			: Show the default console
Parameters		: console - The console to show
Returns			: None
*/
void show_console(console_t * console);

/*
Function name	: clrscr
Purpose			: Clear screen
Parameters		: None
*/
void clrscr(console_t * console);

/*
Function name	: draw_header
Purpose			: Draws process header
Parameters		: None
*/
void draw_header(console_t * console, uchar_t * name);

/*
Function name	: printf
Purpose			: Print string to the screen
Parameters		: string - the string
*/
void printf(console_t * console, const char * string, ...);

/*
Function name	: print_character
Purpose			: Print a character to the screen
Parameters		: video_memory - Pointer to the memory location
				  character - The character to print
*/
void print_character(console_t * console, uchar_t ** video_memory, uchar_t character);

/*
Function name	: remove_character
Purpose			: Removes a character from the screen
Parameters		: console - The working IO console
*/
void remove_character(console_t * console);

/*
Function name	: gotoxy
Purpose			: Go to a specific point on the screen
Return			: TRUE - Success
				  FALSE - Failure
Parameters		: row - The row of the character
				  column - The column of the character
*/
bool_t gotoxy(console_t * console, ushort_t row, ushort_t column);

/*
Function name	: calculate_screen_offset
Purpose			: Calculate the screen of set for a specific row and column
Return			: On success offset of character, on failure 0
Parameters		: row - The row of the character
				  column - The column of the character
*/
ulong_t calculate_screen_offset(ushort_t row, ushort_t column);

/*
Function name	: convert_to_hex
Purpose			: Convert the lower nible of a byte to a printable character
Return			: Printable character
Parameters		: character - The value to convert
*/
char convert_to_hex(char character);

/*
Function name	: convert_number_to_printable
Purpose			: Convert 0-9 number into a printable character
Return			: Printable character
Parameters		: number - The value to convert [0-9]
*/
char convert_number_to_printable(ushort_t number);

/*
Function name	: calculate_screen_bounds
Purpose			: Calculate screen bounds overflow
Parameters		: None
*/
void calculate_screen_bounds(console_t * console);

/*
Function name	: calculate_screen_color
Purpose			: Calculate screen color using the global colors
Parameters		: None
*/
void calculate_screen_color(console_t * console);

/*
Function name	: console_foreground
Purpose			: Set the foreground color
Parameters		: None
*/
void console_foreground(console_t * console, uchar_t color);

/*
Function name	: console_background
Purpose			: Set the background color
Parameters		: None
*/
void console_background(console_t * console, uchar_t color);

/*
Function name	: get_viewing_process
Purpose			: Get the current viewing process
Parameters		: None
*/
process_t * get_viewing_process(void);

#endif

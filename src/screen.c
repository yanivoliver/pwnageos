/*
Handle screen stuff
Author: Shimi G.
*/
#include "common.h"
#include "io.h"
#include "screen.h"

ushort_t g_screen_row = 0;
ushort_t g_screen_column = 0;
ushort_t g_screen_color = SCREEN_DEFAULT_COLOR;
ushort_t g_screen_foreground = SCREEN_FOREGROUND_COLOR;
ushort_t g_screen_background = SCREEN_BACKGROUND_COLOR;

void clrscr()
{
	/* Variables */
	uchar_t * video_memory = NULL;
	ulong_t screen_size = 0;
	ulong_t i = 0;

	/* Set screen memory */
	video_memory = (uchar_t *)SCREEN_SEGMENT;

	/* Set screen size */
	screen_size = SCREEN_ROWS * SCREEN_COLUMNS;

	/* Set all screen to nothing */
	for (i = 0; i < screen_size; i++) {
		/* Set the current letter */
		(*video_memory) = SCREEN_EMPTY_CHARACTER;

		/* Move to the next byte */
		video_memory++;

		(*video_memory) = g_screen_color;

		/* Move to the next byte */
		video_memory++;

		/* Each letter is 2 bytes, so we actualy moved to the next letter */
	}

	/* Set cursor position to 0,0 */
	out(0x3D4, 14);
	out(0x3D5, 0);
	out(0x3D4, 15);
	out(0x3D5, 0);
}

void printf(const char * string, ...)
{
	/* Variables */
	char buffer[30] = {0};
	uchar_t * video_memory_base = NULL;
	uchar_t * video_memory = NULL;
	bool_t special_character = FALSE;
	bool_t format_mode = FALSE;
	va_list arguments = NULL;
	uchar_t working_character = 0;
	ulong_t working_long = 0;
	ulong_t counter = 0;
	ulong_t current_argument = 0;
	ulong_t multiply = 0;
	ulong_t i = 0;
	ulong_t x = 0;
	ulong_t z = 0;

	/* Set the arguments */
	va_start(arguments, string);

	/* Jump over the first argument */
	va_arg(arguments, char *);

	/* Set screen memory */
	video_memory_base = (uchar_t *)SCREEN_SEGMENT;

	/* Offset the screen to the correct location */
	video_memory = video_memory_base;
	video_memory += calculate_screen_offset(g_screen_row, g_screen_column);

	/* Loop all characters */
	i = 0;
	while (CHARACTER_TERMINATOR != string[i]) {
		/* Check if we are in format mode */
		if (TRUE == format_mode) {
			/* Select format mode*/
			switch (string[i]) {
				case 'X':
					/* Print the hex number */
					working_character = va_arg(arguments, char);
					print_character(&video_memory, convert_to_hex(working_character >> 4));
					print_character(&video_memory, convert_to_hex(working_character & 0xF));
					break;
				case 'c':
					/* Print the character */
					working_character = va_arg(arguments, int);
					print_character(&video_memory, working_character);
					break;
				case 'd':
					/* Print the hex number */
					working_long = va_arg(arguments, long);
					x = 0;
					while (0 != working_long) {
						counter = working_long % 10;
						working_long /= 10;
						buffer[x] = convert_number_to_printable(counter);
						x++;
					}

					/* Print the numbers backwards */
					for (z = 1; z <= x; z++) {
						print_character(&video_memory, buffer[x-z]);
					}
					break;
				default:
					/* Its not a known format mode, we probably need to print the % sign */
					print_character(&video_memory, '%');

					/* Special case when this is actualy not a format mode.. so turn it down */
					//format_mode = FALSE;
					break;
			}
		}

		/* If we were in format mode, we need to skip the printing */
		if (TRUE != format_mode) {
			/* Check if its a special character */
			special_character = FALSE;
			switch (string[i]) {
				case '\n':
					/* Set to the start of the next row */
					g_screen_row += 1;
					g_screen_column = 0;

					/* Set the screen position*/
					video_memory = video_memory_base;
					video_memory += calculate_screen_offset(g_screen_row, g_screen_column);

					/* Set as a special character */
					special_character = TRUE;

					/* Check screen bounds */
					calculate_screen_bounds();
					break;
				case '%':
					format_mode = TRUE;
					special_character = TRUE;
					break;
			}

			/* Print it only if its not a special character */
			if (TRUE != special_character) {
				/* Print character */
				print_character(&video_memory, string[i]);
			}
		} else {
			/* Reset the format mode */
			format_mode = FALSE;
		}

		/* Move the pointer to the next character */
		i++;
	}
}

char convert_to_hex(char character)
{
	/* Get only the lower nibble */
	character &= 0xF;

	/* Check if its a [0-9] or [a-f] */
	if (character <= 9) {
		return '0' + character;
	} else {
		return 'A' + (character - 0xA);
	}
}

char convert_number_to_printable(ushort_t number)
{
	/* Check bounderies */
	if (number > 9) {
		return 0;
	}

	/* Convert number */
	return '0' + number;
}

void print_character(uchar_t ** video_memory, uchar_t character)
{
	/* Print the character to the screen */
	(**video_memory) = character;
	(*video_memory) += 1;

	(**video_memory) = g_screen_color;
	(*video_memory) += 1;

	/* Move the screen to the next character */
	g_screen_column += 1;

	/* Recalculate screen bounds */
	calculate_screen_bounds();
}

ulong_t calculate_screen_offset(ushort_t row, ushort_t column)
{
	/* Check the boundry of the requested row and column */
	//if (0 > row || SCREEN_ROWS < row || 0 > column || SCREEN_COLUMNS < column) {
		/* Wrong row or column number. Return function */
	//	return 0;
	//}
	return 2 * (column + (row * SCREEN_COLUMNS));
}

void calculate_screen_bounds()
{
	/* Variables */
	uchar_t * video_memory = NULL;
	ushort_t i = 0;
	ushort_t x = 0;

	/* Set screen memory */
	video_memory = (uchar_t *)SCREEN_SEGMENT;

	/* Check column bound */
	if (SCREEN_COLUMNS <= g_screen_column) {
		/* Set to the start of the next row */
		g_screen_row += 1;
		g_screen_column = 0;
	}

	if (SCREEN_ROWS <= g_screen_row) {
		/* Copy all rows to the rows which are below */
		/* Iterate all rows except the first one which will be overwritten */
		for (i = 1; i < SCREEN_ROWS; i++) {
			/* Iterate all columns */
			/* We have two bytes per column */
			for (x = 0; x < SCREEN_COLUMNS*2; x++) {
				(*(video_memory+x+((i-1)*SCREEN_COLUMNS*2))) = (*(video_memory+x+(i*SCREEN_COLUMNS*2)));
			}
		}

		/* Go to the last line */
		g_screen_row -= 1;

		/* Clear the last line */
		for (i = 0; i < SCREEN_COLUMNS*2; i++) {
			*(video_memory+(g_screen_row*SCREEN_COLUMNS*2)+i) = 0x00;
		}		
	}
}

bool_t gotoxy(ushort_t row, ushort_t column)
{
	/* Check the boundry of the requested row and column */
	//if (0 > row || SCREEN_ROWS < row || 0 > column || SCREEN_COLUMNS < column) {
		/* Wrong row or column number. Return function */
	//	return FALSE;
	//}

	/* Set the row */
	g_screen_row = row;

	/* Set the column */
	g_screen_column = row;

	/* Return success */
	return TRUE;
}

void calculate_screen_color()
{
	/* Calculate the screen color */
	g_screen_color = ((g_screen_background << 4) | g_screen_foreground);
}

void console_foreground(uchar_t color)
{
	/* Set the foreground */
	g_screen_foreground = color;

	/* Recalculate colors */
	calculate_screen_color();
}

void console_background(uchar_t color)
{
	/* Set the foreground */
	g_screen_background = color;

	/* Recalculate colors */
	calculate_screen_color();
}

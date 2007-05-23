/*
Handle screen stuff
Author: Shimi G.
*/
#include "common.h"
#include "schedule.h"
#include "io.h"
#include "memory.h"
#include "string.h"
#include "screen.h"

#define HEADER_BACKGROUND	(0x4)
#define HEADER_FOREGROUND	(0xF)
#define HEADER_SEPERATOR	(0xAF)

//console_t * g_default_console = NULL;	/* Default console if nothing else is found, should be pointing to idle console */
//console_t * g_viewing_console = NULL;	/* The one that viewed currently to the screen */

process_t * g_default_process = NULL;	/* Default console if nothing else is found, should be pointing to idle console */
process_t * g_viewing_process = NULL;	/* The one that viewed currently to the screen */


void show_console(console_t * console)
{
	/* Declare variables */
	process_t * process = NULL;

	/* Check references */
	if (NULL == console) {
		return;
	}

	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Set the default console as viewing console */
	process = get_process_by_console(console);
	g_viewing_process = process;

	/* Update the screen */
	screen_update(console, TRUE);
}

process_t * get_viewing_process(void)
{
	return g_viewing_process;
}

void show_next_console()
{
	/* Declare variables */
	process_t * process = NULL;

	/* Get the first process*/
	process = get_process_by_console(&g_viewing_process->console);
	if (NULL == process) {
		/* Error finding process */
		return;
	}

	/* If its the last process jump to the first process */
	if (NULL == process->next_process) {
		show_console(&get_head_process()->console);
	}
	/* Just go to the next process */
	else {
		show_console(&process->next_process->console);
	}
}

void show_prev_console()
{
	/* Declare variables */
	process_t * process = NULL;
	process_t * process_found = NULL;

	/* Get the first process*/
	process = get_process_by_console(&g_viewing_process->console);
	if (NULL == process) {
		/* Error finding process */
		return;
	}

	/* If its the first process jump to the last one process */
	if (get_head_process() == process) {
		show_console(&get_last_process()->console);
	}
	/* Just go to the previous process */
	else {
		process_found = get_prev_process(process);
		if (NULL != process_found) {
			show_console(&process_found->console);
		}
	}
}

bool_t init_screen(ulong_t process_id)
{
	/* Declare variables */
	process_t * process = NULL;

	/* Get process */
	process = get_process_by_id(process_id);
	if (NULL == process) {
		return FALSE;
	}

	/* Set as default console */
	g_default_process = process;
	g_viewing_process = process;

	/* Draw header */
	//draw_header(process->name);

	/* Invalidate */
	//screen_update(TRUE);

	/* Return success*/
	return TRUE;
}

void screen_update(console_t * console, bool_t forced)
{
	/* Variables */
	uchar_t * video_memory = NULL;

	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Check if viewing console is the working one */
	if (TRUE != forced && &g_viewing_process->console != console) {
		/* We dont need to redraw anything */
		return;
	}

	/* Set video memory */
	video_memory = (uchar_t *)SCREEN_SEGMENT;

	/* Copy the console to the shared screen memory */
	memcpy(video_memory, g_viewing_process->console.screen, SCREEN_ROWS * SCREEN_COLUMNS * 2);
}

void clrscr(console_t * console)
{
	/* Variables */
	uchar_t * video_memory = NULL;
	ulong_t screen_size = 0;
	ulong_t i = 0;

	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Set screen memory */
	//video_memory = (uchar_t *)SCREEN_SEGMENT;
	video_memory = console->screen;

	/* Set screen size */
	screen_size = SCREEN_ROWS * SCREEN_COLUMNS;

	/* Set all screen to nothing */
	for (i = 0; i < screen_size; i++) {
		/* Set the current letter */
		(*video_memory) = SCREEN_EMPTY_CHARACTER;

		/* Move to the next byte */
		video_memory++;

		(*video_memory) = console->color;

		/* Move to the next byte */
		video_memory++;

		/* Each letter is 2 bytes, so we actualy moved to the next letter */
	}

	/* Set cursor position to 0,0 */
	out(0x3D4, 14);
	out(0x3D5, -1);
	out(0x3D4, 15);
	out(0x3D5, -1);

	/* Update the screen */
	screen_update(console, FALSE);
}

void draw_header(console_t * console, uchar_t * name)
{
	/* Declare variables */
	//process_t * process = NULL;
	ulong_t i = 0;
	ulong_t offset_count = 0;
	uchar_t * help = "<F1-Previous> <F2-Next> <F3-Processes>";

	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Get current process */
	//process = get_current_process();

	/* Color the first line */
	for (i = 1; i < SCREEN_COLUMNS*2; i+=2) {
		console->screen[i] = HEADER_BACKGROUND << 4 | HEADER_FOREGROUND;
	}

	/* Get process name */
	offset_count = 4;
	console->screen[2] = HEADER_SEPERATOR;
	for (i = 0; i < strlen(name); i++) {
		console->screen[offset_count] = name[i];
		offset_count += 2;
	}

	/* Get process name */
	offset_count = (SCREEN_COLUMNS-strlen(help)-1)*2;
	console->screen[2] = HEADER_SEPERATOR;
	for (i = 0; i < strlen(help); i++) {
		console->screen[offset_count] = help[i];
		offset_count += 2;
	}
}


void printf(console_t * console, const char * string, ...)
{
	/* Variables */
	char buffer[256] = {0};
	uchar_t * video_memory_base = NULL;
	uchar_t * video_memory = NULL;
	bool_t special_character = FALSE;
	bool_t format_mode = FALSE;
	va_list arguments = NULL;
	uchar_t working_character = 0;
	uchar_t * working_string = NULL;
	ulong_t working_long = 0;
	ulong_t counter = 0;
	ulong_t current_argument = 0;
	ulong_t multiply = 0;
	ulong_t i = 0;
	ulong_t x = 0;
	ulong_t z = 0;

	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Set the arguments */
	va_start(arguments, string);

	/* Jump over the first argument */
	va_arg(arguments, char *);

	/* Set screen memory */
	//video_memory_base = (uchar_t *)SCREEN_SEGMENT;
	video_memory_base = console->screen;

	/* Offset the screen to the correct location */
	video_memory = video_memory_base;
	video_memory += calculate_screen_offset(console->row, console->column);

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
					print_character(console, &video_memory, convert_to_hex(working_character >> 4));
					print_character(console, &video_memory, convert_to_hex(working_character & 0xF));
					break;
				case 'c':
					/* Print the character */
					working_character = va_arg(arguments, int);
					if ('\n' == working_character) {
						/* Set to the start of the next row */
						console->row += 1;
						console->column = 0;

						/* Set the screen position*/
						//video_memory = video_memory_base;
						//video_memory += calculate_screen_offset(console->row, console->column);

						/* Check screen bounds */
						calculate_screen_bounds(console);
					} else {
						print_character(console, &video_memory, working_character);
					}
					break;
				case 's':
					/* Print the character */
					working_string = va_arg(arguments, int);

					/* Iterate string */
					while ('\0' != (*working_string)) {
						/* Set working character */
						working_character = (*working_string);

						/* Check if its a special character */
						if ('\n' == working_character) {
							/* Set to the start of the next row */
							console->row += 1;
							console->column = 0;

							/* Check screen bounds */
							calculate_screen_bounds(console);

							/* Set the screen position*/
							video_memory = video_memory_base;
							video_memory += calculate_screen_offset(console->row, console->column);
						} else {
							/* Print the character */
							print_character(console, &video_memory, working_character);
						}
						
						/* Get next character */
						working_string++;
					}
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
						print_character(console, &video_memory, buffer[x-z]);
					}
					break;
				default:
					/* Its not a known format mode, we probably need to print the % sign */
					print_character(console, &video_memory, '%');

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
					console->row += 1;
					console->column = 0;
					
					/* Check screen bounds */
					calculate_screen_bounds(console);

					/* Set the screen position*/
					video_memory = video_memory_base;
					video_memory += calculate_screen_offset(console->row, console->column);

					/* Set as a special character */
					special_character = TRUE;
					break;
				case '%':
					format_mode = TRUE;
					special_character = TRUE;
					break;
			}

			/* Print it only if its not a special character */
			if (TRUE != special_character) {
				/* Print character */
				print_character(console, &video_memory, string[i]);
			}
		} else {
			/* Reset the format mode */
			format_mode = FALSE;
		}

		/* Move the pointer to the next character */
		i++;
	}

	/* Update the screen */
	screen_update(console, FALSE);
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

void print_character(console_t * console, uchar_t ** video_memory, uchar_t character)
{
	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Print the character to the screen */
	(**video_memory) = character;
	(*video_memory) += 1;

	(**video_memory) = console->color;
	(*video_memory) += 1;

	/* Move the screen to the next character */
	console->column += 1;

	/* Recalculate screen bounds */
	calculate_screen_bounds(console);
}

void remove_character(console_t * console)
{
	/* Declare variables */
	uchar_t * video_memory = NULL;

	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Check if the beginning of the line */
	if (0 == console->column && console->row == console->row_header) {
		return;
	}

	/* Calculate new row and column */
	if (0 == console->column) {
		console->column = SCREEN_COLUMNS;
		console->row -= 1;
	} else {
		console->column -= 1;
	}
	
	/* Get character position */
	video_memory = console->screen + calculate_screen_offset(console->row, console->column);	

	/* Blank the character */
	(*video_memory) = SCREEN_EMPTY_CHARACTER;
	(*(video_memory+1)) = console->color;

	/* Update the screen */
	screen_update(console, FALSE);
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

void calculate_screen_bounds(console_t * console)
{
	/* Variables */
	uchar_t * video_memory = NULL;
	ushort_t i = 0;
	ushort_t x = 0;

	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Set screen memory */
	//video_memory = (uchar_t *)SCREEN_SEGMENT;
	video_memory = console->screen;

	/* Check column bound */
	if (SCREEN_COLUMNS <= console->column) {
		/* Set to the start of the next row */
		console->row += 1;
		console->column = 0;
	}

	if (SCREEN_ROWS <= console->row) {
		/* Copy all rows to the rows which are below */
		/* Iterate all rows except the first one which will be overwritten */
		for (i = 1+console->row_header; i < SCREEN_ROWS; i++) {
			/* Iterate all columns */
			/* We have two bytes per column */
			for (x = 0; x < SCREEN_COLUMNS*2; x++) {
				(*(video_memory+x+((i-1)*SCREEN_COLUMNS*2))) = (*(video_memory+x+(i*SCREEN_COLUMNS*2)));
			}
		}

		/* Go to the last line */
		console->row -= 1;

		/* Clear the last line */
		for (i = 0; i < SCREEN_COLUMNS*2; i++) {
			*(video_memory+(console->row*SCREEN_COLUMNS*2)+i) = 0x00;
		}		
	}
}

bool_t gotoxy(console_t * console, ushort_t row, ushort_t column)
{
	/* Check the boundry of the requested row and column */
	//if (0 > row || SCREEN_ROWS < row || 0 > column || SCREEN_COLUMNS < column) {
		/* Wrong row or column number. Return function */
	//	return FALSE;
	//}

	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Set the row */
	console->row = row;

	/* Set the column */
	console->column = column;

	/* Return success */
	return TRUE;
}

void calculate_screen_color(console_t * console)
{
	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Calculate the screen color */
	console->color = ((console->background << 4) | console->foreground);
}

void console_foreground(console_t * console, uchar_t color)
{
	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Set the foreground */
	console->foreground = color;

	/* Recalculate colors */
	calculate_screen_color(console);
}

void console_background(console_t * console, uchar_t color)
{
	/* Check console variable */
	if (NULL == console) {
		/* Set to defauly console */
		console = &g_default_process->console;
	}

	/* Set the foreground */
	console->background = color;

	/* Recalculate colors */
	calculate_screen_color(console);
}

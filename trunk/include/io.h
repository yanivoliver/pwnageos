/*
IO Header
Author: Shimi G.
*/

#ifndef HEADER_PWNAGE_IO
#define HEADER_PWNAGE_IO

/*
Function name	: gotoxy
Purpose			: Go to a specific point on the screen
Parameters		: row - The row of the character
				  column - The column of the character
*/
uchar_t in(ushort_t port);

/*
Function name	: gotoxy
Purpose			: Go to a specific point on the screen
Parameters		: row - The row of the character
				  column - The column of the character
*/
void out(ushort_t port, uchar_t data);

#endif

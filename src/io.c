/*
IO functions
Author: Shimi G.
*/
#include "common.h"
#include "io.h"

uchar_t in(ushort_t port)
{
	/* Variables */
	uchar_t result = 0;

	/* Inline assembly of a simple in instruction */
	__asm__ ("in %%dx, %%al" : "=a" (result) : "d" (port));
  
	/* Return the result */
	return result;
}


void out(ushort_t port, uchar_t data)
{
	/* Inline assembly of a simple out instruction */
	__asm__ ("out %%al, %%dx" : :"a" (data), "d" (port));
}

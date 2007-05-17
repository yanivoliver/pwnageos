/*
Memory functions
Author: Shimi G.
Date: 04/05/07
*/

#include "common.h"
#include "memory.h"

void memset(void * address, char byte, ulong_t size)
{
	/* Some variables */
	ulong_t i = 0;

	/* Just loop and set the byte */
	for (i = 0; i < size; i++) {
		(*((char *)address)) = byte;
	}
}

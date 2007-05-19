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

void * memcpy(void * destination, void * source, ulong_t size)
{
	/* Declare variables */
	ulong_t i = 0;

	/* Check references */
	if (NULL == destination || NULL == source || 0 == size) {
		return NULL;
	}

	/* Loop amount of size bytes and copy */
	for (i = 0; i < size; i++) {
		(*(((ulong_t *)destination)+i)) = (*(((ulong_t *)source)+i));
	}

	return destination;
}

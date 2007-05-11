/*
Memory header
Author: Shimi G.
*/

#ifndef HEADER_PWNAGE_MEMORY
#define HEADER_PWNAGE_MEMORY

/*
Function name	: memset
Purpose			: Set memory region
Parameters		: address - The memory address
				  byte - The byte to set
				  size - The size of the region
*/
void memset(void * address, char byte, ulong_t size);

#endif

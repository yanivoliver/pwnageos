/*
Memory functions
Author: Shimi G.
Date: 04/05/07
*/

#include "common.h"
#include "gdt.h"
#include "memory.h"
#include "screen.h"

extern void load_gdtr(ushort_t * row);

gdt_entry_t g_gdt_table[NUMBER_OF_GDT_ENTRIES];

#define GDT_DATA_TYPE	(0x02)
#define GDT_CODE_TYPE	(0x0A)
#define GDT_TSS_TYPE	(0x09)

bool_t init_gdt()
{
	/* Declare variables */
	ulong_t index = 0;
	ushort_t limit_base[3] = {0};

	/* Clear the entries */
	memset(g_gdt_table, '\0', GDT_ENTRY_SIZE*NUMBER_OF_GDT_ENTRIES);

	/* Set kernel code segment */
	set_entry_bounds(1, 0x0000, 0xFFFF);
	g_gdt_table[1].db_bit = 1;
	g_gdt_table[1].dpl = KERNEL_PRIVILEGE;
	g_gdt_table[1].present = 1;
	g_gdt_table[1].system = 1;
	g_gdt_table[1].type = GDT_CODE_TYPE;
	g_gdt_table[1].reserved = 0;
	g_gdt_table[1].granularity = GDT_GRANULARITY_PAGE;
	g_gdt_table[1].used = 1;

	/* Set kernel data segment */
	set_entry_bounds(2, 0x0000, 0xFFFF);
	g_gdt_table[2].db_bit = 1;
	g_gdt_table[2].dpl = KERNEL_PRIVILEGE;
	g_gdt_table[2].present = 1;
	g_gdt_table[2].system = 1;
	g_gdt_table[2].type = GDT_DATA_TYPE;
	g_gdt_table[2].reserved = 0;
	g_gdt_table[2].granularity = GDT_GRANULARITY_PAGE;
	g_gdt_table[2].used = 1;

	/* Set the gdtr */
	limit_base[0] = GDT_ENTRY_SIZE*NUMBER_OF_GDT_ENTRIES;
	limit_base[1] = (ulong_t)g_gdt_table & 0xFFFF;
	limit_base[2] = (ulong_t)g_gdt_table >> 16;

	/* Load the new gdtr */	
	load_gdtr(limit_base);

	return TRUE;
}

void set_entry_bounds(ulong_t index, ulong_t address, ulong_t size)
{
	/* Set 32 bits of address */
	g_gdt_table[index].base_low = address & 0xFFFFFF;
	g_gdt_table[index].base_high = (address >> 24) & 0xFF;

	/* Set 20 bits of size */
	g_gdt_table[index].size_low = size & 0xFFFF;
	g_gdt_table[index].size_high = (size >> 16) & 0x0F;
}

bool_t allocate_gdt_entry(ulong_t * index)
{
	/* Declare variables */
	ulong_t i = 0;

	/* Check references */
	if (NULL == index) {
		/* Bad references */
		return FALSE;
	}

	/* Loop */
	for (i = 1; i < NUMBER_OF_GDT_ENTRIES; i++) {
		if (TRUE != g_gdt_table[i].used) {
			/* Found the entry. Set the address */
			(*index) = i;

			/* Set entry as unavailble */
			g_gdt_table[i].used = TRUE;

			/* Return true */
			return TRUE;
		}
	}

	/* No entry found. Return false */
	return FALSE;
}

void free_gdt_entry(ulong_t index)
{
	/* Clear the entry */
	memset(&g_gdt_table[index], '\0', GDT_ENTRY_SIZE);
}

ushort_t segment_selector(protection_level_t rpl, bool_t is_gdt, ulong_t index)
{
	/* Declare variables */
	ushort_t selector = 0;

	/* Set the selector */
	selector |= rpl & 0x3;
	selector |= ((TRUE == is_gdt)? 0 : 1) << 2;
	selector |= (index & 0x1FFF) << 3;

	/* Return */
	return selector;	
}

gdt_entry_t * get_gdt_entry(ulong_t index)
{
	/* Check bounds */
	if (NUMBER_OF_GDT_ENTRIES <= index)
	{
		/* Out-of-bound entry */
		return NULL;
	}

	/* Check if its a used entry */
	if (TRUE != g_gdt_table[index].used)
	{
		/* Unused entry */
		return NULL;
	}

	/* Valid entry */
	return &g_gdt_table[index];
}


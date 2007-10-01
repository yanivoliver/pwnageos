/*
TSS Managment
Author: Shimi G.
Date: 11/05/07
*/

#include "common.h"
#include "tss.h"
#include "gdt.h"
#include "memory.h"
#include "screen.h"

/* Extern from kernel-lowlevel */
extern void load_tr(ushort_t row);

/* Single tss entry */
tss_t g_tss;
ulong_t g_tss_entry_index = 0;

tss_entry_t g_tss_table[NUMBER_OF_TSS_ENTRIES] __attribute__ ((aligned (16)));

bool_t init_tss()
{
	/* Declare variables */
	ulong_t tss_entry_index = 0;
	gdt_entry_t * tss_entry = NULL;

	/* Clear the tss table */
	memset(&g_tss_table[0], '\0', sizeof(tss_entry_t)*NUMBER_OF_TSS_ENTRIES);

	/* Success */
	return TRUE;
}

bool_t allocate_tss_entry(ulong_t tss_entry_index, tss_t ** tss)
{
	/* Declare variables */
	ulong_t i = 0;

	/* Check references */
	if (NULL == tss) {
		/* Bad references */
		return FALSE;
	}

	/* Loop */
	for (i = 1; i < NUMBER_OF_TSS_ENTRIES; i++) {
		if (TRUE != g_tss_table[i].used) {
			/* Set entry as unavailble */
			g_tss_table[i].used = TRUE;
			g_tss_table[i].tss_entry_index = tss_entry_index;

			/* Set the OUT variable */
			(*tss) = &g_tss_table[i].tss;

			/* Return true */
			return TRUE;
		}
	}

	/* No entry found. Return false */
	return FALSE;
}

ulong_t create_tss()
{
	/* Declare variables */
	ulong_t tss_entry_index = 0;
	gdt_entry_t * tss_entry = NULL;
	tss_t * tss = NULL;

	/* Set gdt entry of the tss record */
	if (TRUE != allocate_gdt_entry(&tss_entry_index))
	{
		/* Error allocating gdt entry */
		return 0;
	}

	/* Get gdt */
	tss_entry = get_gdt_entry(tss_entry_index);
	if (NULL == tss_entry) {
		/* Error allocating gdt entry */
		return 0;
	}

	/* Allocate new tss */
	if (TRUE != allocate_tss_entry(tss_entry_index, &tss))
	{
		/* Error allocating tss entry */
		return 0;
	}

	/* Set tss entry */
	set_entry_bounds(tss_entry_index, (ulong_t)tss, sizeof(tss_t));
	tss_entry->db_bit = 0;
	tss_entry->dpl = KERNEL_PRIVILEGE;
	tss_entry->present = 1;
	tss_entry->system = 0;
	tss_entry->type = TSS_TYPE_AVAILABLE;
	tss_entry->reserved = 0;
	tss_entry->granularity = GDT_GRANULARITY_BYTES;

	/* Return the gtd entry number */
	return tss_entry_index;
}

void set_tss_available(ushort_t tss_entry_index)
{
	/* Declare variables */
	gdt_entry_t * tss_entry = NULL;
	
	/* Get gdt */
	tss_entry = get_gdt_entry(tss_entry_index);
	if (NULL == tss_entry)
	{
		/* Error allocating gdt entry */
		return;
	}

	/* Set the type */
	tss_entry->type = TSS_TYPE_AVAILABLE;

	return;
}

tss_t * get_tss(ulong_t tss_entry_index)
{
	/* Declare variables */
	gdt_entry_t * tss_entry = NULL;
	ulong_t i = 0;

	/* Get gdt */
	tss_entry = get_gdt_entry(tss_entry_index);
	if (NULL == tss_entry) {
		/* Error allocating gdt entry */
		return;
	}

	/* Find the tss */
	for (i = 1; i < NUMBER_OF_TSS_ENTRIES; i++) {
		if (g_tss_table[i].tss_entry_index == tss_entry_index) {
			return &g_tss_table[i].tss;
		}
	}

	/* Return the tss */
	return NULL;
}
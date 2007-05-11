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

extern void load_tr(ushort_t row);

/* TSS Entry */
tss_t g_tss;

bool_t init_tss()
{
	/* Declare variables */
	ulong_t tss_entry_index = 0;
	gdt_entry_t * tss_entry = NULL;

	/* Set gdt entry of the tss record */
	if (TRUE != allocate_gdt_entry(&tss_entry_index))
	{
		/* Error allocating gdt entry */
		return FALSE;
	}

	/* Get gdt */
	tss_entry = get_gdt_entry(tss_entry_index);
	if (NULL == tss_entry)
	{
		/* Error allocating gdt entry */
		return FALSE;
	}

	/* Set tss entry */
	set_entry_bounds(tss_entry_index, (ulong_t)&g_tss, sizeof(tss_t));
	tss_entry->db_bit = 0;
	tss_entry->dpl = KERNEL_PRIVILEGE;
	tss_entry->present = 1;
	tss_entry->system = 0;
	tss_entry->type = TSS_TYPE_AVAILABLE;
	tss_entry->reserved = 0;
	tss_entry->granularity = GDT_GRANULARITY_BYTES;

	load_tr( segment_selector(KERNEL_PRIVILEGE, TRUE, tss_entry_index) );

	/* Success */
	return TRUE;
}

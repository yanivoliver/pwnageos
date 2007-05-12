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
#include "schedule.h"

/* Extern from kernel-lowlevel */
extern void load_tr(ushort_t row);

/* TSS Entry */
tss_t g_tss;
ulong_t g_tss_entry_index = 0;

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

	/* Set global tss index */
	g_tss_entry_index = tss_entry_index;

	/* Set tss entry */
	set_entry_bounds(tss_entry_index, (ulong_t)&g_tss, sizeof(tss_t));
	tss_entry->db_bit = 0;
	tss_entry->dpl = USER_PRIVILEGE;
	tss_entry->present = 1;
	tss_entry->system = 0;
	tss_entry->type = TSS_TYPE_AVAILABLE;
	tss_entry->reserved = 0;
	tss_entry->granularity = GDT_GRANULARITY_BYTES;

	/* Set tss values */
	memset(&g_tss, '\0', sizeof(tss_t));
	g_tss.ss_0 = KERNEL_DS;
	g_tss.esp_0 = 0x80000;
	g_tss.ss_1 = KERNEL_DS;
	g_tss.esp_1 = 0x80000;
	g_tss.ss_2 = KERNEL_DS;
	g_tss.esp_2 = 0x50000;
	g_tss.es = USER_DS | 0x0003;
	g_tss.cs = USER_CS | 0x0003;
	g_tss.ds = USER_DS | 0x0003;
	g_tss.ss = USER_DS | 0x0003;
	g_tss.fs = USER_DS | 0x0003;
	g_tss.gs = USER_DS | 0x0003;
	g_tss.eip = idle;
	g_tss.esp = 0x80000;
	g_tss.eflags = 0x0202;

	/* Load tss */
	load_tr( segment_selector(USER_PRIVILEGE, TRUE, tss_entry_index) );

	/* Success */
	return TRUE;
}

void set_tss_available()
{
	/* Declare variables */
	gdt_entry_t * tss_entry = NULL;

	/* Get gdt */
	tss_entry = get_gdt_entry(g_tss_entry_index);
	if (NULL == tss_entry)
	{
		/* Error allocating gdt entry */
		return;
	}

	/* Set the type */
	tss_entry->type = TSS_TYPE_AVAILABLE;
}

void print_tss()
{
	printf("[%X%X%X%X]", g_tss.esp_0);
}
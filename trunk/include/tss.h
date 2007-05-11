/*
GDT Managment
Author: Shimi G.
*/

#ifndef HEADER_PWNAGE_TSS
#define HEADER_PWNAGE_TSS

/* TSS Types */
#define TSS_TYPE_AVAILABLE	(9)
#define TSS_TYPE_BUSY		(11)

/* Entry for a gdt */
typedef struct tss_rec {
	ushort_t	back_link;
	ushort_t	reserved_0;
	uint_t		esp_0;
	ushort_t	ss_0;
	ushort_t	reserved_1;
	uint_t		esp_1;
	ushort_t	ss_1;
	ushort_t	reserved_2;
	uint_t		esp_2;
	ushort_t	ss_2;
	ushort_t	reserved_3;
	uint_t		cr_3;
	uint_t		eip;
	uint_t		eflags;
	uint_t		eax;
	uint_t		ecx;
	uint_t		edx;
	uint_t		ebx;
	uint_t		esp;
	uint_t		ebp;
	uint_t		esi;
	uint_t		edi;
	ushort_t	es;
	ushort_t	reserved_4;
	ushort_t	cs;
	ushort_t	reserved_5;
	ushort_t	ss;
	ushort_t	reserved_6;
	ushort_t	ds;
	ushort_t	reserved_7;
	ushort_t	fs;
	ushort_t	reserved_8;
	ushort_t	gs;
	ushort_t	reserved_9;
	ushort_t	ldt;
	ushort_t	reserved_10;
	ushort_t	debug_trap : 1		__attribute__((packed));
	ushort_t	reserved_11 : 15	__attribute__((packed));
	ushort_t	io_map;
} tss_t;

#endif

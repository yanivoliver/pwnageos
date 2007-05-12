/*
GDT Managment
Author: Shimi G.
*/

#ifndef HEADER_PWNAGE_GDT
#define HEADER_PWNAGE_GDT

/* Number of gdt entries */
#define NUMBER_OF_GDT_ENTRIES		(16)
#define GDT_ENTRY_SIZE				(8)

/* Granularity types */
#define GDT_GRANULARITY_BYTES	(0)
#define GDT_GRANULARITY_PAGE	(1)

/* Entry for a gdt */
typedef struct gdt_entry_rec {
	ushort_t size_low;
	ulong_t base_low : 24	__attribute__((packed));
    uint_t type : 4			__attribute__((packed));
    uint_t system : 1		__attribute__((packed));
    uint_t dpl : 2			__attribute__((packed));
    uint_t present : 1		__attribute__((packed));
    uint_t size_high : 4	__attribute__((packed));
    uint_t used : 1			__attribute__((packed));
    uint_t reserved : 1		__attribute__((packed));
    uint_t db_bit : 1		__attribute__((packed));
    uint_t granularity : 1	__attribute__((packed));
    uchar_t base_high;
} gdt_entry_t;

/*
Function name	: init_gdt
Purpose			: Initialize basic gdt table
Parameters		: None
*/
bool_t init_gdt();

/*
Function name	: allocate_gdt_entry
Purpose			: Allocate a gdt entry
Parameters		: OUT index - Entry number
*/
bool_t allocate_gdt_entry(ulong_t * index);

/*
Function name	: free_gdt_entry
Purpose			: Clear gdt entry
Parameters		: index - Entry number
*/
void free_gdt_entry(ulong_t index);

/*
Function name	: set_entry_bounds
Purpose			: Set the base and limit of an entry
Parameters		: index - Entry number
				  address - Entry address
				  size - Entry size
*/
void set_entry_bounds(ulong_t index, ulong_t address, ulong_t size);

/*
Function name	: segment_selector
Purpose			: Get a segment selector
Parameters		: rpl - Requested protection level
				  is_gdt - Idicates whether to select gdt or ldt
				  index - INDEX of gdt entry
*/
ushort_t segment_selector(protection_level_t rpl, bool_t is_gdt, ulong_t index);

/*
Function name	: segment_selector
Purpose			: Get a segment selector
Parameters		: index - Requested gdt entry index
*/
gdt_entry_t * get_gdt_entry(ulong_t index);

#endif


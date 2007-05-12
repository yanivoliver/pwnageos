/*
Common definitions
Author: Shimi G.
*/

#ifndef HEADER_PWNAGE_COMMON
#define HEADER_PWNAGE_COMMON

/* Define null */
#define NULL	(void *)0

/* Define short names for known types */
typedef unsigned short ushort_t;
typedef unsigned long ulong_t;
typedef unsigned char uchar_t;
typedef unsigned int uint_t;
typedef unsigned short bool_t;

/* Define true and false values */
#define TRUE	(1)
#define FALSE	(0)

/* Defing elipsis arguments */
typedef char * va_list;
#define va_start(LIST, ARGUMENT) (LIST) = (char *)(&(ARGUMENT))
#define va_arg(LIST, TYPE) (LIST += sizeof(TYPE), (*((TYPE *)(LIST-sizeof(TYPE)))))

/* Code segment and data segment */
#define KERNEL_CS	(1<<3)
#define KERNEL_DS	(2<<3)
#define USER_CS		(3<<3)
#define USER_DS		(4<<3)


/* Priviliges */
typedef enum protection_level_e {
	KERNEL_PRIVILEGE	= 0,
	USER_PRIVILEGE		= 3
} protection_level_t;

#endif

/*
Floppy driver
Author: Shimi G.
*/

#ifndef HEADER_PWNAGE_FLOPPY
#define HEADER_PWNAGE_FLOPPY

typedef struct floppy_drive_rec {
	uchar_t type;
} floppy_drive_t __attribute__ ((aligned));

typedef enum floppy_registers_e {
	FLOPPY_DIGITAL_OUTPUT = 2,
	FLOPPY_DIGITAL_INPUT = 7,
	FLOPPY_MAIN_STATUS = 4,
	FLOPPY_CONFIG_CONTROL = 7,
	FLOPPY_DATA = 5
} floppy_registers_t __attribute__ ((aligned));

typedef enum floppy_data_rate_e {
	FLOPPY_DATA_RATE_500 = 0,
	FLOPPY_DATA_RATE_300 = 1,
	FLOPPY_DATA_RATE_250 = 2,
	FLOPPY_DATA_RATE_1000 = 3
} floppy_data_rate_t __attribute__ ((aligned));

typedef enum floppy_command_e {
	FLOPPY_COMMAND_SPECIFY = 3,
	FLOPPY_COMMAND_SENSE_INTERRUPT = 8,
	FLOPPY_COMMAND_RECALIBRATE = 7,
	FLOPPY_COMMAND_SEEK = 15,
	FLOPPY_COMMAND_READ = 6,
	FLOPPY_COMMAND_WRITE = 5
} floppy_command_t __attribute__ ((aligned));

typedef struct floppy_main_status_register_rec {
	uchar_t drive_a_busy : 1			__attribute__((packed));
	uchar_t drive_b_busy : 1			__attribute__((packed));
	uchar_t drive_c_busy : 1			__attribute__((packed));
	uchar_t drive_d_busy : 1			__attribute__((packed));
	uchar_t command_in_progress : 1		__attribute__((packed));
	uchar_t non_dma : 1					__attribute__((packed));
	uchar_t system_direction : 1		__attribute__((packed));
	uchar_t ready_for_transfer : 1		__attribute__((packed));
} floppy_main_status_register_t;

typedef struct floppy_sector_status_rec {
	uchar_t status_0;
	uchar_t status_1;
	uchar_t status_2;
	uchar_t cylinder;
	uchar_t head;
	uchar_t sector;
	uchar_t sector_length;
} floppy_sector_status_t;

typedef struct floppy_sense_interrupt_result_rec {
	uchar_t status_0;
	uchar_t present_cylinder;
} floppy_sense_interrupt_result_t;

#define FLOPPY_A				(0)
#define FLOPPY_B				(1)

#define FLOPPY_RETRIES_SEND			(10000)
#define FLOPPY_RETRIES_RECEIVE		(10000)
#define FLOPPY_RETRIES_SEEK			(3)
#define FLOPPY_RETRIES_READ			(10)

#define FLOPPY_PORT(FLOPPY_INDEX, FLOPPY_REGISTER)	(floppy_base_address[(FLOPPY_INDEX)] + (FLOPPY_REGISTER))

/*
Function name	: init_floppy
Purpose			: Initialize floppy drive
Parameters		: None
*/
bool_t init_floppy();

void floppy_list_drives();

#endif


/*
Memory functions
Author: Shimi G.
Date: 04/05/07
*/

#include "common.h"
#include "floppy.h"
#include "irq.h"
#include "io.h"

#define FLOPPY_DELAY_LOOPS	(50000)

static floppy_drive_t g_floppy_drives[2] = {0};
static floppy_sense_interrupt_result_t g_sense_interrupt = {0};
static ushort_t floppy_base_address[] = {0x3F0, 0x370};
static bool_t g_floppy_pending = FALSE;
static bool_t g_floppy_motor = FALSE;
static char * g_floppy_types[6] =	{	"Not found",
										"5.25\", 350kb",
										"5.25\", 1.2mb",
										"3.5\", 750kb",
										"3.5\", 1.44mb",
										"3.5\", 2.88mb"};

void floppy_handler(ushort_t irq, registers_t * registers)
{
	/* Clear pending */
	g_floppy_pending = FALSE;
}

void floppy_delay()
{
	/* Declare variables */
	ulong_t i = 0;

	/* Loop */
	for (i = 0; i < FLOPPY_DELAY_LOOPS; i++) {
	}
}

void floppy_delay_on_interrupt()
{
	/* Loop */
	while (TRUE == g_floppy_pending) {
	}
}

floppy_main_status_register_t floppy_read_main_status_register()
{
	/* Declare variables */
	floppy_main_status_register_t msr = {0};
	uchar_t msr_raw = 0;

	/* Read from the port */
	msr_raw = in(FLOPPY_PORT(FLOPPY_A, FLOPPY_MAIN_STATUS));

	/* Copy data */
	memcpy(&msr, &msr_raw, sizeof(msr_raw));

	/* Return the msr */
	return msr;
}

bool_t floppy_reset()
{	
	/* Declare variabels */
	ulong_t i = 0;

	/* Set floppy as pending */
	g_floppy_pending = TRUE;

	/* Reset floppy */
	out(FLOPPY_PORT(FLOPPY_A, FLOPPY_DIGITAL_OUTPUT), 0);

	/* Wait for floppy to be resetted */
	floppy_delay();

	/* Set floppy mode to 500bit\s */
	out(FLOPPY_PORT(FLOPPY_A, FLOPPY_DIGITAL_INPUT), 0);

	/* Enable floppy again */
	out(FLOPPY_PORT(FLOPPY_A, FLOPPY_DIGITAL_OUTPUT), 0xC);

	/* Wait for floppy to be setted */
	floppy_delay();

	/* Check if interrupt fired */
	if (TRUE == g_floppy_pending) {
		return FALSE;
	}

	/* Sense for 4 times */
	for (i = 0; i < 4; i++) {
		if (TRUE != floppy_command_sense_interrupt(NULL)) {
			return FALSE;
		}
	}

	/* Return true */
	return TRUE;
}

void floppy_set_data_rate(floppy_data_rate_t data_rate)
{
	/* Set the data rate */
	out(FLOPPY_PORT(FLOPPY_A, FLOPPY_CONFIG_CONTROL), data_rate);
}

bool_t floppy_motor_on()
{
	/* Check motor state */
	if (TRUE == g_floppy_motor) {
		/* Its already on */
		return TRUE;
	}

	/* Turn floppy motor */
	out(FLOPPY_PORT(FLOPPY_A, FLOPPY_DIGITAL_OUTPUT), 0x1C);

	floppy_delay();

	return TRUE;
}

bool_t floppy_motor_off()
{
	/* Check motor state */
	if (TRUE != g_floppy_motor) {
		/* Its already on */
		return TRUE;
	}

	/* Turn floppy motor */
	out(FLOPPY_PORT(FLOPPY_A, FLOPPY_DIGITAL_OUTPUT), 0xC);

	floppy_delay();

	return TRUE;

}

bool_t floppy_command_sense_interrupt()
{
	/* Declare variables */
	uchar_t byte_0 = 0;
	uchar_t byte_1 = 0;
	uchar_t byte_2 = 0;

	/* Set command byte */
	byte_0 = FLOPPY_COMMAND_SENSE_INTERRUPT;

	/* Send byte 0 */
	if (TRUE != floppy_send_byte(byte_0)) {
		return FALSE;
	}

	/* Receive result byte 0 */
	if (TRUE != floppy_receive_byte(&byte_1)) {
		return FALSE;
	}

	/* Receive result byte 1 */
	if (TRUE != floppy_receive_byte(&byte_2)) {
		return FALSE;
	}

	/* Send the result */
	g_sense_interrupt.status_0 = byte_1;
	g_sense_interrupt.present_track = byte_2;

	/* Success */
	return TRUE;
}

bool_t floppy_command_seek(uchar_t head, uchar_t cylinder)
{
	/* Declare variables */
	uchar_t byte_0 = 0;
	uchar_t byte_1 = 0;
	uchar_t byte_2 = 0;
	bool_t send_success = FALSE;

	/* Set command byte */
	byte_0 = FLOPPY_COMMAND_SEEK;

	/* Set head and drive byte */
	byte_1 = ((head & 0x1) << 2) | (FLOPPY_A & 0x3);

	/* Set cylinder */
	byte_2 = cylinder;

	while (TRUE != send_success) {
		/* Send byte 0 */
		if (TRUE != floppy_send_byte(byte_0)) {
			return FALSE;
		}

		/* Send byte 1 */
		if (TRUE != floppy_send_byte(byte_1)) {
			return FALSE;
		}

		/* Send byte 2 */
		if (TRUE != floppy_send_byte(byte_2)) {
			return FALSE;
		}

		floppy_delay_on_interrupt();
		send_success = floppy_command_sense_interrupt();
	}

	/* Success */
	return TRUE;
}

bool_t floppy_command_specify(uchar_t stepping_rate, uchar_t head_load, uchar_t head_unload, bool_t dma)
{
	/* Declare variables */
	uchar_t byte_0 = 0;
	uchar_t byte_1 = 0;
	uchar_t byte_2 = 0;

	/* Set command byte */
	byte_0 = FLOPPY_COMMAND_SPECIFY;

	/* Set stepping rate and head unload */
	byte_1 = ((stepping_rate & 0xF) << 4) | (head_unload & 0xF);

	/* Set head load and dma */
	byte_2 = ((head_load & 0x7F) << 1) | (dma & 0x1);

	/* Send byte 0 */
	if (TRUE != floppy_send_byte(byte_0)) {
		return FALSE;
	}

	/* Send byte 1 */
	if (TRUE != floppy_send_byte(byte_1)) {
		return FALSE;
	}

	/* Send byte 2 */
	if (TRUE != floppy_send_byte(byte_2)) {
		return FALSE;
	}

	/* Success */
	return TRUE;
}

bool_t floppy_command_recalibrate()
{
	/* Declare variables */
	uchar_t byte_0 = 0;
	uchar_t byte_1 = 0;
	bool_t send_success = FALSE;

	/* Set command byte */
	byte_0 = FLOPPY_COMMAND_RECALIBRATE;

	/* Set stepping rate and head unload */
	byte_1 = FLOPPY_A;

	/* Send bytes */
	while (TRUE != send_success) {
		/* Set to pending mode */
		g_floppy_pending = TRUE;

		/* Send byte 0 */
		if (TRUE != floppy_send_byte(byte_0)) {
			return FALSE;
		}

		/* Send byte 1 */
		if (TRUE != floppy_send_byte(byte_1)) {
			return FALSE;
		}

		floppy_delay_on_interrupt();
		send_success = floppy_command_sense_interrupt();
	}

	/* Success */
	return TRUE;
}

bool_t floppy_send_byte(uchar_t data)
{
	/* Declare variables */
	ulong_t i = 0;
	floppy_main_status_register_t msr = {0};

	/* Iterate until out of retries */
	for (i = 0; i < FLOPPY_RETRIES_SEND; i++) {
		/* Get msr */
		msr = floppy_read_main_status_register();

		/* Check if we are valid to read from it */
		if (msr.ready_for_transfer) {
			out(FLOPPY_PORT(FLOPPY_A, FLOPPY_DATA), data);
			return TRUE;
		}

		/* Delay */
		floppy_delay();
	}

	/* Out of retries */
	return FALSE;
}

bool_t floppy_receive_byte(uchar_t * data)
{
	/* Declare variables */
	ulong_t i = 0;
	uchar_t byte = 0;
	floppy_main_status_register_t msr = {0};
	
	/* Check references */
	if (NULL == data) {
		return FALSE;
	}

	/* Iterate until out of retries */
	for (i = 0; i < FLOPPY_RETRIES_RECEIVE; i++) {
		/* Get msr */
		msr = floppy_read_main_status_register();

		/* Check if we are valid to read from it */
		if (msr.ready_for_transfer && msr.system_direction) {
			byte = in(FLOPPY_PORT(FLOPPY_A, FLOPPY_DATA));
			(*data) = byte;
			return TRUE;
		}

		/* Delay */
		floppy_delay();
	}

	/* Out of retries */
	return FALSE;
}

void floppy_list_drives()
{
	/* Print the drives */
	printf(NULL, "    %cDrive A - %s\n", 0xAF, g_floppy_types[ g_floppy_drives[FLOPPY_A].type ]);
	printf(NULL, "    %cDrive B - %s\n", 0xAF, g_floppy_types[ g_floppy_drives[FLOPPY_B].type ]);
}

bool_t init_floppy()
{
	/* Declare variables */
	uchar_t floppy_detect = 0;

	/* Setup irq */
	install_irq_handler(6, floppy_handler);
	enable_irq(6);

	/* Set pending parameter */
	g_floppy_pending = FALSE;

	/* Set motor state */
	g_floppy_motor = FALSE;

	/* Check which floppy is alive */
	out(0x70, 0x10);
	floppy_detect = in(0x71);

	g_floppy_drives[FLOPPY_A].type = (floppy_detect>>4) & 0xF;
	g_floppy_drives[FLOPPY_B].type = floppy_detect & 0xF;

	/* Reset drive */
	if (TRUE != floppy_reset()) {
		/* Error resetting floppy */
		return FALSE;
	}

	/* Turn floppy motor on */
	if (TRUE != floppy_motor_on()) {
		/* Error turnning floppy on */
		return FALSE;
	}

	/* Set the data rate */
	floppy_set_data_rate(FLOPPY_DATA_RATE_500);

	/* Specify times */
	/* SRT - Delay time for the head between tracks 
			 Calculation: SRT = 16 - (delay_seconds * data_rate(=500000) / 500)
			 Recommended: 8ms, SRT = 8
	   HLT - Delay between the head movement and when its ready
			 Calculation: HLT = delay_seconds / 1000 * data_rate(=500000)
			 Recommended: 10ms, HLT = 5
	   HUT - Delay after head finished moving
			 Calculation: HUT = delay_seconds / 8000 * data_rate(=500000)
			 Recommended: 240ms, HUT = 15
	*/
	if (TRUE != floppy_command_specify(8, 5, 15, FALSE)) {
		return FALSE;
	}

	/* Recalibrate */
	floppy_command_recalibrate();

	floppy_command_seek(0, 1);

	//printf(NULL, "-> %X", g_sense_interrupt.present_track);

	return TRUE;
}

/*
Memory functions
Author: Shimi G.
Date: 04/05/07
*/

#include "common.h"
#include "dma.h"
#include "io.h"

#define DMA_CHANNELS	(8)

dma_channel_t g_dma_channel[] = {	{0x0A, 0x0B, 0x0C, 0x87, 0x00, 0x01},
									{0x0A, 0x0B, 0x0C, 0x83, 0x02, 0x03},
									{0x0A, 0x0B, 0x0C, 0x81, 0x04, 0x05},
									{0x0A, 0x0B, 0x0C, 0x82, 0x06, 0x07},
									{0xD4, 0xD6, 0xD8, 0x8F, 0xC0, 0xC2},
									{0xD4, 0xD6, 0xD8, 0x8B, 0xC4, 0xC6},
									{0xD4, 0xD6, 0xD8, 0x89, 0xC8, 0xCA},
									{0xD4, 0xD6, 0xD8, 0x8A, 0xCC, 0xCE}	};

ulong_t g_dma_address[] = {	0x0,
							0x10000,
							0x20000,
							0x30000,
							0x40000,
							0x50000,
							0x60000,
							0x70000,
							0x80000,
							0x90000,
							0xA0000,
							0xB0000,
							0xC0000,
							0xD0000,
							0xE0000,
							0xF0000	};

bool_t init_dma()
{
	return TRUE;
}

bool_t dma_start(uchar_t channel_number, void * address, ulong_t length, dma_operation_t operation)
{
	/* Declare variables */
	dma_channel_t channel = {0};

	/* Check references */
	if (NULL == address) {
		return FALSE;
	}

	/* Check channel */
	if (channel_number >= DMA_CHANNELS) {
		return FALSE;
	}

	/* Set the channel information */
	channel = g_dma_channel[channel_number];

	/* Disable the dma so we could configure it */
	out(channel.mask, 0x04 | channel_number);

	/* Clear data transfers */
	out(channel.clear, 0x0);

	/* Set the dma mode */
	out(channel.mode, operation | channel_number);

	/* Set the address to zero */
	out(channel.address, 0x0);

	/* Set the page according to the page addreses layout which is page == channel */
	out(channel.page, channel_number);

	/* Set the length of the data (low the high) */
	out(channel.count, length & 0xFF);
	out(channel.count, (length & 0xFF00) >> 8);

	/* Enable the dma */
	out(channel.mask, channel_number);

	return TRUE;
}

bool_t dma_stop(uchar_t channel_number)
{
	/* Declare variables */
	dma_channel_t channel = {0};

	/* Check channel */
	if (channel_number >= DMA_CHANNELS) {
		return FALSE;
	}

	/* Set the channel information */
	channel = g_dma_channel[channel_number];

	/* Disable the dma so we could configure it */
	out(channel.mask, 0x04 | channel_number);

	/* Clear data transfers */
	out(channel.clear, 0x0);

	/* Enable the dma */
	out(channel.mask, channel_number);
}

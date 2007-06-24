/*
Floppy driver
Author: Shimi G.
*/

#ifndef HEADER_PWNAGE_DMA
#define HEADER_PWNAGE_DMA

typedef struct dma_block_rec {
	uchar_t channel;
	ulong_t address;
	ulong_t length;
} dma_block_t;

typedef struct dma_channel_rec {
	uchar_t mask;
	uchar_t mode;
	uchar_t clear;
	uchar_t page;
	uchar_t address;
	uchar_t count;
} dma_channel_t;

typedef enum dma_operation_e {
	DMA_READ	= 8,
	DMA_WRITE	= 4
} dma_operation_t;

/*
Function name	: init_dma
Purpose			: Initialize dma chips
Parameters		: None
*/
bool_t init_dma();

bool_t dma_start(uchar_t channel_number, void * address, ulong_t length, dma_operation_t operation);

bool_t dma_stop(uchar_t channel_number);

#endif


#pragma once

typedef struct fec_parms *fec_code_t;

/*
 * create a new encoder, returning a descriptor. This contains k,n and
 * the encoding matrix.
 * n is the number of data blocks + fec blocks (matrix height)
 * k is just the data blocks (matrix width)
 */
void fec_init(void);

void fec_encode(unsigned int blockSize,
		unsigned char **data_blocks,
		unsigned int nrDataBlocks,
		unsigned char **fec_blocks,
		unsigned int nrFecBlocks);

void fec_decode(unsigned int blockSize,
		unsigned char **data_blocks,
		unsigned int nr_data_blocks,
		unsigned char **fec_blocks,
		unsigned int *fec_block_nos,
		unsigned int *erased_blocks,
		unsigned short nr_fec_blocks  /* how many blocks per stripe */);

void fec_print(fec_code_t code, int width);

void fec_license(void);


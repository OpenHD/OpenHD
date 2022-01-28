#ifndef FEC_2_H
#define FEC_2_H

#ifdef __cplusplus
extern "C" {
#endif

//#define PROFILE

typedef struct fec_parms *fec_code_t;
typedef unsigned char gf;

/*
 * create a new encoder, returning a descriptor. This contains k,n and
 * the encoding matrix.
 * n is the number of data blocks + fec blocks (matrix height)
 * k is just the data blocks (matrix width)
 */
void fec_init(void);

// don't bother to understand that c style crap, look at FEC.hpp
void fec_encode(unsigned int blockSize,
                const gf **data_blocks,
                unsigned int nrDataBlocks,
                gf **fec_blocks,
                unsigned int nrFecBlocks);

// don't bother to understand that c style crap, look at FEC.hpp
void fec_decode(unsigned int blockSize,
                gf **data_blocks,
                unsigned int nr_data_blocks,
                gf **fec_blocks,
                const unsigned int fec_block_nos[],
                const unsigned int erased_blocks[],
                unsigned short nr_fec_blocks  /* how many blocks per stripe */);

void fec_print(fec_code_t code, int width);

void fec_license(void);

#ifdef PROFILE
void printDetail(void);
#endif

#ifdef __cplusplus
}
#endif

#endif //FEC_2_H
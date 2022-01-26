#ifndef FEC_2_H
#define FEC_2_H

//#ifdef __cplusplus
//extern "C" {
//#endif

//#define PROFILE

#include <stdint.h>

typedef uint8_t gf;


/**
 * Consti10 - fec_init is not needed anymore - the gf256 values are precomputed and stored in the header(s) of optimized -
 * and the block size / n data / n fec blocks is variable (note: for each sequence of blocks, the encode / decode params need to match though)
 */
//void fec_init(void);

/**
 * @param blockSize size of each block (all blocks must have the same size)
 * @param data_blocks array of pointers to the memory of the data blocks
 * @param nrDataBlocks how many data blocks
 * @param fec_blocks array of pointers to the memory of the fec blocks (generated)
 * @param nrFecBlocks how many fec blocks to generate
 */
void fec_encode(unsigned int blockSize,
                const gf **data_blocks,
                unsigned int nrDataBlocks,
                gf **fec_blocks,
                unsigned int nrFecBlocks);

/**
 *
 * @param blockSize size of each block
 * @param data_blocks array of pointers to the memory of the data blocks. Missing areas will be filled
 * @param nr_data_blocks how many data blocks (available and missing)
 * @param fec_blocks array of pointers to the memory of the fec blocks
 * @param fec_block_nos indices of the received fec blocks
 * @param erased_blocks indices of the erased / missing data blocks that will be reconstructed
 * @param nr_fec_blocks how many data blocks were erased - need at least this many fec blocks.
 */
void fec_decode(unsigned int blockSize,
                gf **data_blocks,
                unsigned int nr_data_blocks,
                gf **fec_blocks,
                const unsigned int fec_block_nos[],
                const unsigned int erased_blocks[],
                unsigned short nr_fec_blocks  /* how many blocks per stripe */);

void fec_license(void);


#ifdef PROFILE
void printDetail(void);
#endif

//#ifdef __cplusplus
//}
//#endif

#include <vector>


/**
 * @param fragmentSize size of each fragment in this block
 * @param primaryFragments list of pointers to memory for primary fragments
 * @param secondaryFragments list of pointers to memory for secondary fragments (fec fragments)
 * Using the data from @param primaryFragments constructs as many secondary fragments as @param secondaryFragments holds
 */
void fec_encode2(unsigned int fragmentSize,
                const std::vector<const uint8_t*>& primaryFragments,
                const std::vector<uint8_t*>& secondaryFragments);

/**
 * @param fragmentSize size of each fragment in this block
 * @param primaryFragments list of pointers to memory for primary fragments. Must be same size as used for fec_encode()
 * @param indicesMissingPrimaryFragments list of the indices of missing primary fragments.
 * Example: if @param indicesMissingPrimaryFragments contains 2, the 3rd primary fragment is missing
 * @param secondaryFragmentsReceived list of pointers to memory for secondary fragments (fec fragments). Must not be same size as used for fec_encode(), only MUST contain "enough" secondary fragments
 * @param indicesOfSecondaryFragmentsReceived list of the indices of secondaryFragments that are used to reconstruct missing primary fragments.
 * Example: if @param indicesOfSecondaryFragmentsReceived contains {0,2}, the first secondary fragment has the index 0, and the second secondary fragment has the index 2
 * When this call returns, all missing primary fragments (gaps) have been filled / reconstructed
 */
void fec_decode2(unsigned int fragmentSize,
                const std::vector<uint8_t*>& primaryFragments,
                const std::vector<unsigned int>& indicesMissingPrimaryFragments,
                const std::vector<uint8_t*>& secondaryFragmentsReceived,
                const std::vector<unsigned int>& indicesOfSecondaryFragmentsReceived);

// Test the (optimized) galois field math
void test_gf();

// Test the fec encoding & reconstructing step
void test_fec();


#endif //FEC_2_H
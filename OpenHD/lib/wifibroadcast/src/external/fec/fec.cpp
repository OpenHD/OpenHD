/*
 * fec.c -- forward error correction based on Vandermonde matrices
 * 980624
 * (C) 1997-98 Luigi Rizzo (luigi@iet.unipi.it)
 * (C) 2001 Alain Knaff (alain@knaff.lu)
 * (C) 2022 Constantin Geier (optimize using libmoepgf source code)
 *
 * Portions derived from code by Phil Karn (karn@ka9q.ampr.org),
 * Robert Morelos-Zaragoza (robert@spectra.eng.hawaii.edu) and Hari
 * Thirumoorthy (harit@spectra.eng.hawaii.edu), Aug 1995
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */


/*
 * The following parameter defines how many bits are used for
 * field elements. The code only supports 8.
 */
#define GF_BITS  8	/* code over GF(2^GF_BITS) - DO NOT CHANGE*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include "fec.h"
/**
 * Include our optimized GF256 math functions - since FEC mostly boils down to "Galois field" mul / madd on big memory blocks
 * this is the most straight forward optimization, and it really speeds up the code by a lot (see paper and my benchmark results)
 * The previous optimization by Alain Knaff used a lookup table. This optimization is still used as a backup, but faster options exists
 * depending on the architecture the code is running on.
 */
#include "gf_optimized//gf256_optimized_include.h"
#include "gf_simple/gf_simple.h"

/*
 * stuff used for testing purposes only
 */

#ifdef	TEST
#define DEB(x)
#define DDB(x) x
#define	DEBUG	0	/* minimal debugging */

#include <sys/time.h>
#define DIFF_T(a,b) \
	(1+ 1000000*(a.tv_sec - b.tv_sec) + (a.tv_usec - b.tv_usec) )

#define TICK(t) \
	{struct timeval x ; \
	gettimeofday(&x, NULL) ; \
	t = x.tv_usec + 1000000* (x.tv_sec & 0xff ) ; \
	}
#define TOCK(t) \
	{ u_long t1 ; TICK(t1) ; \
	  if (t1 < t) t = 256000000 + t1 - t ; \
	  else t = t1 - t ; \
	  if (t == 0) t = 1 ;}

u_long ticks[10];	/* vars for timekeeping */
#else
#define DEB(x)
#define DDB(x)
#define TICK(x)
#define TOCK(x)
#endif /* TEST */


/**
 * Consti10 - the original implementation supported variable GF_BITS values. However, with the optimizations
 * and the HW developments since 1997 it makes no sense to use anything lower than GF(2^8). And the optimizations by
 * Alain Knaff made GF_BITS == 8 an requirement anyways
 */
#if (GF_BITS != 8)
#error "GF_BITS must be 8"
#endif


#define SWAP(a,b,t) {t tmp; tmp=a; a=b; b=tmp;}
/*
 * invert_mat() takes a matrix and produces its inverse
 * k is the size of the matrix.
 * (Gauss-Jordan, adapted from Numerical Recipes in C)
 * Return non-zero if singular.
 */
DEB( int pivloops=0; int pivswaps=0 ; /* diagnostic */)
static int
invert_mat(gf *src, int k)
{
    gf c, *p ;
    int irow, icol, row, col, i, ix ;

    int error = 1 ;
    int indxc[k];
    int indxr[k];
    int ipiv[k];
    gf id_row[k];

    memset(id_row, 0, k*sizeof(gf));
    DEB( pivloops=0; pivswaps=0 ; /* diagnostic */ )
    /*
     * ipiv marks elements already used as pivots.
     */
    for (i = 0; i < k ; i++)
        ipiv[i] = 0 ;

    for (col = 0; col < k ; col++) {
        gf *pivot_row ;
        /*
         * Zeroing column 'col', look for a non-zero element.
         * First try on the diagonal, if it fails, look elsewhere.
         */
        irow = icol = -1 ;
        if (ipiv[col] != 1 && src[col*k + col] != 0) {
            irow = col ;
            icol = col ;
            goto found_piv ;
        }
        for (row = 0 ; row < k ; row++) {
            if (ipiv[row] != 1) {
                for (ix = 0 ; ix < k ; ix++) {
                    DEB( pivloops++ ; )
                    if (ipiv[ix] == 0) {
                        if (src[row*k + ix] != 0) {
                            irow = row ;
                            icol = ix ;
                            goto found_piv ;
                        }
                    } else if (ipiv[ix] > 1) {
                        fprintf(stderr, "singular matrix\n");
                        goto fail ;
                    }
                }
            }
        }
        if (icol == -1) {
            fprintf(stderr, "XXX pivot not found!\n");
            goto fail ;
        }
        found_piv:
        ++(ipiv[icol]) ;
        /*
         * swap rows irow and icol, so afterwards the diagonal
         * element will be correct. Rarely done, not worth
         * optimizing.
         */
        if (irow != icol) {
            for (ix = 0 ; ix < k ; ix++ ) {
                SWAP( src[irow*k + ix], src[icol*k + ix], gf) ;
            }
        }
        indxr[col] = irow ;
        indxc[col] = icol ;
        pivot_row = &src[icol*k] ;
        c = pivot_row[icol] ;
        if (c == 0) {
            fprintf(stderr, "singular matrix 2\n");
            goto fail ;
        }
        if (c != 1 ) { /* otherwhise this is a NOP */
            /*
             * this is done often , but optimizing is not so
             * fruitful, at least in the obvious ways (unrolling)
             */
            DEB( pivswaps++ ; )
            c = gf256_inverse( c ) ;
            pivot_row[icol] = 1 ;
            for (ix = 0 ; ix < k ; ix++ )
                pivot_row[ix] = gf256_mul(c, pivot_row[ix] );
        }
        /*
         * from all rows, remove multiples of the selected row
         * to zero the relevant entry (in fact, the entry is not zero
         * because we know it must be zero).
         * (Here, if we know that the pivot_row is the identity,
         * we can optimize the addmul).
         */
        id_row[icol] = 1;
        if (memcmp(pivot_row, id_row, k*sizeof(gf)) != 0) {
            for (p = src, ix = 0 ; ix < k ; ix++, p += k ) {
                if (ix != icol) {
                    c = p[icol] ;
                    p[icol] = 0 ;
                    gf256_madd_optimized(p, pivot_row, c, k );
                }
            }
        }
        id_row[icol] = 0;
    } /* done all columns */
    for (col = k-1 ; col >= 0 ; col-- ) {
        if (indxr[col] <0 || indxr[col] >= k)
            fprintf(stderr, "AARGH, indxr[col] %d\n", indxr[col]);
        else if (indxc[col] <0 || indxc[col] >= k)
            fprintf(stderr, "AARGH, indxc[col] %d\n", indxc[col]);
        else
        if (indxr[col] != indxc[col] ) {
            for (row = 0 ; row < k ; row++ ) {
                SWAP( src[row*k + indxr[col]], src[row*k + indxc[col]], gf) ;
            }
        }
    }
    error = 0 ;
    fail:
    return error ;
}


/**
 * Simplified re-implementation of Fec-Bourbon
 *
 * Following changes have been made:
 *  1. Avoid unnecessary copying of block data.
 *  2. Avoid expliciting matrixes, if we are only going to use one row
 *     anyways
 *  3. Pick coefficients of Vandermonde matrix in such a way as to get
 *     a "nicer" systematic matrix, such as for instance the following:
 *           1 0 0 0 0 0 0 0
 *           0 1 0 0 0 0 0 0
 *           0 0 1 0 0 0 0 0
 *           0 0 0 1 0 0 0 0
 *           0 0 0 0 1 0 0 0
 *           0 0 0 0 0 1 0 0
 *           0 0 0 0 0 0 1 0
 *           0 0 0 0 0 0 0 1
 *           a b c d e f g h
 *           b a d c f e h g
 *           c d a b g h e f
 *           d c b a h g f e
 *
 *     This makes it easyer on processor cache, because we keep on reusing the
 *     same small part of the multiplication table.
 *     The trick to obtain this is to use k=128 and n=256. Use x=col for
 *     top matrix (rather than exp(col-1) as the original did). This makes
 *     the "inverting" polynom to be the following (coefficients of col
 *     col of inverse of top Vandermonde matrix)
 *
 *                _____
 *                 | |
 *   P   =  K      | |    (x - i)
 *    col    col   | |
 *             0 < i < 128 &&
 *               i != col
 *
 *   K_col must be chosen such that P_col(col) = 1, thus
 *
 *                  1
 *           ---------------
 *   K    =       _____
 *    col          | |
 *                 | |    (col - i)
 *                 | |
 *             0 < i < 128 &&
 *               i != col
 *
 *     For obvious reasons, all (col-i)'s are different foreach i (because
 *     col constant). Moreoveover, none has the high bit set (because both
 *     col and i have high bit unset and +/- is really a xor). Moreover
 *     0 is not among them (because i != col). This means that we calculate
 *     the product of all values for 1 to 0x7f, and we have eliminated
 *     dependancy on col. K_col can be written just k.
 *
 *     Which make P_col resolves to:
 *               _____
 *                | |
 *     P   =  K   | |    (x - i)
 *      col       | |
 *             0 < i < 128
 *           -------------------
 *              (x-col)
 *
 *     When evaluating this for any x > 0x80, the following thing happens
 *     to the numerator: all (x-i) are different for i, and have high bit
 *     set. Thus, the set of top factors are all values from 0x80 to 0xff,
 *     and the numerator becomes independant from x (as long as x & 0x80 = 0)
 *     Thus, P_col(x) = L / (x-col)
 *     In the systematic matrix value on [row,col] is P_col(row) = L/(row-col)
 *     To simplify we multiply each bottom row by 1/L (which is a simple
 *     scaling operation, and should not affect invertibility of any partial
 *     matrix contained therein), and we get S[row,col] = 1/(row-col)
 *     Benefits of all this:
 *       - no complicated encoding matrix to compute (it's just the inverse
 *       table!)
 *       - cache efficiency when multiplying blocks, because we get to
 *       reuse the same coefficients. Probability of mult table already in
 *       cache increases.
 *     Downside:
 *       - less flexibility: we can for instance not do 240/200, because
 *       200 is more than 128, and using this technique we unfortunately
 *       limited number of data blocks to 128 instead of 256 as would be
 *       possible otherwise
 */



/* We do the matrix multiplication columns by column, instead of the
 * usual row-by-row, in order to capitalize on the cache freshness of
 * each data block . The data block only needs to be fetched once, and
 * can be used to be addmull'ed into all FEC blocks at once. No need
 * to worry about evicting FEC blocks from the cache: those are so
 * few (typically, 4 or 8) that they will fit easily in the cache (even
 * in the L2 cache...)
 */
void fec_encode(unsigned int blockSize,
                const gf **data_blocks,
                unsigned int nrDataBlocks,
                gf **fec_blocks,
                unsigned int nrFecBlocks)

{
    unsigned int blockNo; /* loop for block counter */
    unsigned int row, col;

    assert(nrDataBlocks <= 128);
    assert(nrFecBlocks <= 128);

    if(!nrDataBlocks)
        return;

    for(row=0; row < nrFecBlocks; row++)
        gf256_mul_optimized(fec_blocks[row], data_blocks[0], gf256_inverse(128 ^ row), blockSize);

    for(col=129, blockNo=1; blockNo < nrDataBlocks; col++, blockNo ++) {
        for(row=0; row < nrFecBlocks; row++)
            gf256_madd_optimized(fec_blocks[row], data_blocks[blockNo],
                   gf256_inverse(row ^ col),
                   blockSize);
    }
}

/**
 * Reduce the system by substracting all received data blocks from FEC blocks
 * This will allow to resolve the system by inverting a much smaller matrix
 * (with size being number of blocks lost, rather than number of data blocks
 * + fec)
 */
static inline void reduce(unsigned int blockSize,
                          gf **data_blocks,
                          unsigned int nr_data_blocks,
                          gf **fec_blocks,
                          const unsigned int fec_block_nos[],
                          const unsigned int erased_blocks[],
                          unsigned short nr_fec_blocks)
{
    int erasedIdx=0;
    unsigned int col;

    /* First we reduce the code vector by substracting all known elements
     * (non-erased data packets) */
    for(col=0; col<nr_data_blocks; col++) {
        if(erasedIdx < nr_fec_blocks && erased_blocks[erasedIdx] == col) {
            erasedIdx++;
        } else {
            gf *src = data_blocks[col];
            int j;
            for(j=0; j < nr_fec_blocks; j++) {
                int blno = fec_block_nos[j];
                gf256_madd_optimized(fec_blocks[j],src,gf256_inverse(blno^col^128),blockSize);
            }
        }
    }

    assert(nr_fec_blocks == erasedIdx);
}

#ifdef PROFILE
static long long rdtsc(void)
{
    unsigned long low, hi;
    asm volatile ("rdtsc" : "=d" (hi), "=a" (low));
    return ( (((long long)hi) << 32) | ((long long) low));
}

long long reduceTime = 0;
long long resolveTime =0;
long long invTime =0;
#endif

/**
 * Resolves reduced system. Constructs "mini" encoding matrix, inverts
 * it, and multiply reduced vector by it.
 */
static inline void resolve(int blockSize,
                           gf **data_blocks,
                           gf **fec_blocks,
                           const unsigned int fec_block_nos[],
                           const unsigned int erased_blocks[],
                           short nr_fec_blocks)
{
#ifdef PROFILE
    long long begin;
#endif
    /* construct matrix */
    int row;
    gf matrix[nr_fec_blocks*nr_fec_blocks];
    int ptr;
    int r;

    /* we pick the submatrix of code that keeps colums corresponding to
     * the erased data blocks, and rows corresponding to the present FEC
     * blocks. This is the matrix by which we would need to multiply the
     * missing data blocks to obtain the FEC blocks we have */
    for(row = 0, ptr=0; row < nr_fec_blocks; row++) {
        int col;
        int irow = 128 + fec_block_nos[row];
        /*assert(irow < fec_blocks+128);*/
        for(col = 0; col < nr_fec_blocks; col++, ptr++) {
            int icol = erased_blocks[col];
            matrix[ptr] = gf256_inverse(irow ^ icol);
        }
    }

#ifdef PROFILE
    begin = rdtsc();
#endif
    r=invert_mat(matrix, nr_fec_blocks);
#ifdef PROFILE
    invTime += rdtsc()-begin;
#endif

    if(r) {
        int col;
        fprintf(stderr,"Pivot not found\n");
        fprintf(stderr, "Rows: ");
        for(row=0; row<nr_fec_blocks; row++)
            fprintf(stderr, "%d ", 128 + fec_block_nos[row]);
        fprintf(stderr, "\n");
        fprintf(stderr, "Columns: ");
        for(col = 0; col < nr_fec_blocks; col++, ptr++)
            fprintf(stderr, "%d ", erased_blocks[col]);
        fprintf(stderr, "\n");
        assert(0);
    }

    /* do the multiplication with the reduced code vector */
    for(row = 0, ptr=0; row < nr_fec_blocks; row++) {
        int col;
        gf *target = data_blocks[erased_blocks[row]];
        gf256_mul_optimized(target,fec_blocks[0],matrix[ptr++],blockSize);
        for(col = 1; col < nr_fec_blocks;  col++,ptr++) {
            gf256_madd_optimized(target,fec_blocks[col],matrix[ptr],blockSize);
        }
    }
}

void fec_decode(unsigned int blockSize,
                gf **data_blocks,
                unsigned int nr_data_blocks,
                gf **fec_blocks,
                const unsigned int fec_block_nos[],
                const unsigned int erased_blocks[],
                unsigned short nr_fec_blocks)
{
#ifdef PROFILE
    long long begin;
    long long end;
#endif

#ifdef PROFILE
    begin = rdtsc();
#endif
    reduce(blockSize, data_blocks, nr_data_blocks,
           fec_blocks, fec_block_nos,  erased_blocks, nr_fec_blocks);
#ifdef PROFILE
    end = rdtsc();
    reduceTime += end - begin;
    begin = end;
#endif
    resolve(blockSize, data_blocks,
            fec_blocks, fec_block_nos, erased_blocks,
            nr_fec_blocks);
#ifdef PROFILE
    end = rdtsc();
    resolveTime += end - begin;
    printDetail();
#endif
}


#ifdef PROFILE
void printDetail(void) {
    fprintf(stderr, "red=%9lld\nres=%9lld\ninv=%9lld\n",
	    reduceTime, resolveTime, invTime);
}
#endif


void fec_license(void)
{
    fprintf(stderr,
            "   wifibroadcast and its FEC code are free software\n"
            "\n"
            "   you can redistribute wifibroadcast core functionality and/or\n"
            "   it them under the terms of the GNU General Public License as\n"
            "   published by the Free Software Foundation; either version 2 of\n"
            "   the License.\n"
            "\n"
            "   This program is distributed in the hope that it will be useful,\n"
            "   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
            "   GNU General Public License for more details.\n"
            "\n"
            "   You should have received a copy of the GNU General Public License\n"
            "   along with this program; see the file COPYING.\n"
            "   If not, write to the Free Software Foundation, Inc.,\n"
            "   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n"
            "\n"
            "the FEC code is covered by the following license:\n"
            "fec.c -- forward error correction based on Vandermonde matrices\n"
            "980624\n"
            "(C) 1997-98 Luigi Rizzo (luigi@iet.unipi.it)\n"
            "(C) 2001 Alain Knaff (alain@knaff.lu)\n"
            "(C) 2022 Constantin Geier (optimize using libmoepgf source code)\n"
            "\n"
            "Portions derived from code by Phil Karn (karn@ka9q.ampr.org),\n"
            "Robert Morelos-Zaragoza (robert@spectra.eng.hawaii.edu) and Hari\n"
            "Thirumoorthy (harit@spectra.eng.hawaii.edu), Aug 1995\n"
            "\n"
            "Redistribution and use in source and binary forms, with or without\n"
            "modification, are permitted provided that the following conditions\n"
            "are met:\n"
            "\n"
            "1. Redistributions of source code must retain the above copyright\n"
            "   notice, this list of conditions and the following disclaimer.\n"
            "2. Redistributions in binary form must reproduce the above\n"
            "   copyright notice, this list of conditions and the following\n"
            "   disclaimer in the documentation and/or other materials\n"
            "   provided with the distribution.\n"
            "\n"
            "THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND\n"
            "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,\n"
            "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A\n"
            "PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS\n"
            "BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,\n"
            "OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"
            "PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,\n"
            "OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
            "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR\n"
            "TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT\n"
            "OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY\n"
            "OF SUCH DAMAGE.\n"
    );
    exit(0);
}

// ---------------------------------- C++ code ------------------------------------------------------------
#include <algorithm>
#include <vector>
#include <iostream>

namespace FUCK{
    static void fillBufferWithRandomData(std::vector<uint8_t>& data){
        const std::size_t size=data.size();
        for(std::size_t i=0;i<size;i++){
            data[i] = rand() % 255;
        }
    }
    std::vector<uint8_t> createRandomDataBuffer(const ssize_t sizeBytes){
        std::vector<uint8_t> buf(sizeBytes);
        fillBufferWithRandomData(buf);
        return buf;
    }
    void assertVectorsEqual(const std::vector<uint8_t>& sb,const std::vector<uint8_t>& rb){
        assert(sb.size()==rb.size());
        const int result=memcmp (sb.data(),rb.data(),sb.size());
        if(result!=0){
            std::cout<<"memcpmp returned "<<result<<"\n";
            assert(true);
        }
        assert(result==0);
    }
    std::vector<std::vector<uint8_t>> createRandomDataBuffers(const std::size_t nBuffers, const std::size_t sizeB){
        std::vector<std::vector<uint8_t>> buffers;
        for(std::size_t i=0;i<nBuffers;i++){
            buffers.push_back(createRandomDataBuffer(sizeB));
        }
        return buffers;
    }
    template<typename Container>
    static std::vector<const uint8_t*> convertToP(const std::vector<Container>& buffs){
        std::vector<const uint8_t*> ret;
        for(const auto& buf:buffs){
            ret.push_back(buf.data());
        }
        return ret;
    }
    template<typename Container>
    static std::vector<uint8_t*> convertToP(std::vector<Container>& buffs){
        std::vector<uint8_t*> ret;
        for(auto& buf:buffs){
            ret.push_back(buf.data());
        }
        return ret;
    }
    // create a indices list for the decodeable case of loosing @param nDroppedDataPackets
    // aka this method first takes (nDataPackets-nDroppedDataPackets) data packets
    // then takes as many fec packets as were lost
    static std::vector<int> createDecodeableIndexList(const int nDataPackets, const int nFecPackets, const int nDroppedDataPackets){
        assert(nFecPackets>=nDroppedDataPackets);
        const int nReceivedDataPackets=nDataPackets-nDroppedDataPackets;
        std::vector<int> indices;
        for(int i=0;i<nReceivedDataPackets;i++){
            indices.push_back(i);
        }
        for(int i=nDataPackets;i<nDataPackets+nDroppedDataPackets;i++){
            indices.push_back(i);
        }
        return indices;
    }
    // calculate binomial coefficient
    // taken from https://www.tutorialspoint.com/binomial-coefficient-in-cplusplus
    static int binomialCoefficients(int n, int k) {
        if (k == 0 || k == n) return 1;
        return binomialCoefficients(n - 1, k - 1) + binomialCoefficients(n - 1, k);
    }
    // code taken from https://www.geeksforgeeks.org/make-combinations-size-k/
    // slightly modified to fit my needs
    void makeCombiUtil(std::vector<std::vector<int>>& ans,std::vector<int>& tmp, int n, int left, int k){
        if (k == 0) {
            ans.push_back(tmp);
            return;
        }
        for (int i = left; i <= n; ++i){
            tmp.push_back(i);
            makeCombiUtil(ans, tmp, n, i + 1, k - 1);
            tmp.pop_back();
        }
    }
    // create all combinations of size k of numbers
    // from [0..n[
    std::vector<std::vector<int> > makeCombi(int n, int k){
        std::vector<std::vector<int>> ans;
        std::vector<int> tmp;
        makeCombiUtil(ans, tmp, n-1, 0, k);
        return ans;
    }
    // create all permutations of recoverable scenarios of received data and fec packets
    // returns as many lists of indices as there are permutations
    static std::vector<std::vector<int>> createAllDecodablePermutations(const int nDataPackets,const int nFecPackets){
        // The task is as following: given the set I is all the indices from 0...(nDataPackets+nFecPackets-1)
        // Create all permutations of taking nDatapackets indices from this list
        const int nDataAndFecPackets=nDataPackets+nFecPackets;
        auto result= makeCombi(nDataAndFecPackets,nDataPackets);
        assert(result.size()==binomialCoefficients(nDataAndFecPackets,nDataPackets));
        return result;
    }
}


// see header for documentation
void fec_encode2(unsigned int fragmentSize,
                const std::vector<const uint8_t*>& primaryFragments,
                const std::vector<uint8_t*>& secondaryFragments){
    fec_encode(fragmentSize, (const gf**)primaryFragments.data(), primaryFragments.size(), (gf**)secondaryFragments.data(), secondaryFragments.size());
}
void fec_decode2(unsigned int fragmentSize,
                const std::vector<uint8_t*>& primaryFragments,
                const std::vector<unsigned int>& indicesMissingPrimaryFragments,
                const std::vector<uint8_t*>& secondaryFragmentsReceived,
                const std::vector<unsigned int>& indicesOfSecondaryFragmentsReceived){
    for(const auto& idx:indicesMissingPrimaryFragments){
        assert(idx<primaryFragments.size());
    }
    //This assertion is not always true - as an example,you might have gotten FEC secondary packets 0 and 4, but these 2 are enough to perform the fec step.
    //Then packet index 0 is inside @param secondaryFragmentsReceived at position 0, but packet index 4 at position 1
    //for(const auto& idx:indicesOfSecondaryFragmentsReceived){
    //    assert(idx<secondaryFragmentsReceived.size());
    //}
    assert(indicesMissingPrimaryFragments.size() <= indicesOfSecondaryFragmentsReceived.size());
    assert(indicesMissingPrimaryFragments.size() == secondaryFragmentsReceived.size());
    assert(secondaryFragmentsReceived.size() == indicesOfSecondaryFragmentsReceived.size());
    fec_decode(fragmentSize, (gf**)primaryFragments.data(), primaryFragments.size(), (gf**)secondaryFragmentsReceived.data(),
               (unsigned int*)indicesOfSecondaryFragmentsReceived.data(), (unsigned int*)indicesMissingPrimaryFragments.data(), indicesMissingPrimaryFragments.size());
}

// see header for documentation
template <class Container1,class Container2>
void fec_encode3(unsigned int fragmentSize,const std::vector<Container1>& primaryFragments,
                 std::vector<Container2>& secondaryFragments){
    fec_encode2(fragmentSize,FUCK::convertToP(primaryFragments),FUCK::convertToP(secondaryFragments));
}
template <class Container1,class Container2>
void fec_decode3(unsigned int fragmentSize,
                 std::vector<Container1>& primaryFragments,
                 const std::vector<unsigned int>& indicesMissingPrimaryFragments,
                 std::vector<Container2>& secondaryFragmentsReceived,
                 const std::vector<unsigned int>& indicesOfSecondaryFragmentsReceived){
    fec_decode2(fragmentSize,FUCK::convertToP(primaryFragments),indicesMissingPrimaryFragments,
                FUCK::convertToP(secondaryFragmentsReceived),indicesOfSecondaryFragmentsReceived);
}


/**
 * Testing the fec encoding and decoding is quite difficult due to the many permutations of either losing data packets,
 * fec packets, or both. This test works as follows:
 * 1) Use the data packets to create @param nFecPackets fec protection packets
 * 2) go through @param receivedDataOrFecPacketsIndices. Write received data and/or fec packets.
 * 3) perform the reconstruction step
 * 4) make sure that after the reconstruction process the content of the data packets matches the input data packets contents.
 * @param dataPackets the data packets whose content will be used to perform the test
 * @param nFecPackets n of fec packets to create
 * @param receivedDataOrFecPacketsIndices indices of received data and fec packets. Note that the valid range for these indices starts at 0 and
 * ends at (nDataPackets+nFecPackets-1). E.g the FEC packet indices don't loopUntilError at 0, like in the c-style code.
 */
static void test_fec_encode_and_decode(const std::vector<std::vector<uint8_t>>& dataPackets, const int nFecPackets,
                                       const std::vector<int>& receivedDataOrFecPacketsIndices){
    const int nDataPackets=dataPackets.size();
    const int packetSize=dataPackets.at(0).size();
    // all data packets need to have the same size
    for(const auto& packet:dataPackets){
        assert(packetSize==packet.size());
    }
    // we need enough received data and fec packet indices
    // else it is not recoverable
    assert(receivedDataOrFecPacketsIndices.size()>=nDataPackets);
    // check the indices were not messed up
    for(const auto& idx:receivedDataOrFecPacketsIndices){
        assert(idx<dataPackets.size()+nFecPackets);
    }
    // allocate memory for the fec packets
    std::vector<std::vector<uint8_t>> fecPackets(nFecPackets,std::vector<uint8_t>(packetSize));
    assert(fecPackets.size()==nFecPackets);
    // encode data packets, store in fec packets
    fec_encode3(packetSize,dataPackets,fecPackets);
    // FEC will fill the not received data packets
    std::vector<std::vector<uint8_t>> fullyReconstructedDataPackets(nDataPackets,std::vector<uint8_t>(packetSize));
    std::vector<unsigned int> erasedDataPacketsIndices;
    // write as many data packets as we have "received"
    // and  mark the rest as missing
    for(int i=0;i<nDataPackets;i++){
        const bool received=std::find(receivedDataOrFecPacketsIndices.begin(),receivedDataOrFecPacketsIndices.end(),i) !=
                receivedDataOrFecPacketsIndices.end();
        if(received){
            memcpy(fullyReconstructedDataPackets[i].data(),dataPackets[i].data(),packetSize);
        }else{
            erasedDataPacketsIndices.push_back(i);
        }
    }
    assert(fullyReconstructedDataPackets.size()==nDataPackets);
    // Write the "received" FEC packets and store their indices for later
    std::vector<std::vector<uint8_t>> receivedFecPackets;
    std::vector<unsigned int> receivedFecPacketsIndices;
    for(int i=nDataPackets;i<nDataPackets+nFecPackets;i++){
        const bool received=std::find(receivedDataOrFecPacketsIndices.begin(),receivedDataOrFecPacketsIndices.end(),i) != receivedDataOrFecPacketsIndices.end();
        if(received){
            const int fecIdx=i-nDataPackets;
            receivedFecPackets.push_back(fecPackets[fecIdx]);
            receivedFecPacketsIndices.push_back(fecIdx);
        }
    }
    // perform the (reconstructing) fec step
    fec_decode3(packetSize,
                // data packets (missing will be filled)
               fullyReconstructedDataPackets,
                erasedDataPacketsIndices,
                // fec packets (used for reconstruction)
                receivedFecPackets,
                receivedFecPacketsIndices
                );
    // make sure everything was reconstructed properly
    for(int i=0;i<nDataPackets;i++){
        FUCK::assertVectorsEqual(dataPackets[i],fullyReconstructedDataPackets[i]);
        //std::cout<<i<<"\n";
    }
    //std::cout<<"SUCCESS: N data packets:"<<nDataPackets<<" N fec packets:"<<nFecPackets<<" N lost&reconstructed data packets:"<<nLostDataPackets<<"\n";
}

/**
 * This test doesn't test all permutations, but rather drops @param nLostDataPackets data packets
 * and then receives exactly this many FEC packets
 */
static void test_fec_encode_and_decode_simple(const int nDataPackets, const int nFecPackets, const int packetSize, const int nLostDataPackets){
    // create our data packets
    const auto dataPackets=FUCK::createRandomDataBuffers(nDataPackets,packetSize);
    assert(dataPackets.size()==nDataPackets);
    const auto indices=FUCK::createDecodeableIndexList(nDataPackets,nFecPackets,nLostDataPackets);
    assert(indices.size()==nDataPackets);
    test_fec_encode_and_decode(dataPackets,nFecPackets,indices);
}
/**
 * Test all permutations of received data and FEC packets that are still recoverable
 */
static void test_fec_encode_and_decode_all_permutations(const int nDataPackets,const int nFecPackets,const int packetSize){
    // create our data packets
    const auto dataPackets=FUCK::createRandomDataBuffers(nDataPackets,packetSize);
    assert(dataPackets.size()==nDataPackets);
    // build the indices lists
    const auto permutations=FUCK::createAllDecodablePermutations(nDataPackets,nFecPackets);
    for(const auto& permutation:permutations){
        test_fec_encode_and_decode(dataPackets,nFecPackets,permutation);
    }
    std::cout<<"Tested all permutations for k:"<<nDataPackets<<" n:"<<nFecPackets<<"\n";
}

void test_fec(){
    gf256_print_optimization_method();
    std::cout<<"Testing FEC reconstruction:\n";
    // test all packet sizes from [1,2048] with fec 8:2 and 9:3
    for(int packetSize=1;packetSize<2048;packetSize++){
        test_fec_encode_and_decode_simple(8,2,packetSize,1);
        test_fec_encode_and_decode_simple(8,2,packetSize,2);
        test_fec_encode_and_decode_simple(9,3,packetSize,1);
        test_fec_encode_and_decode_simple(9,3,packetSize,2);
        test_fec_encode_and_decode_simple(9,3,packetSize,3);
    }
    // for a couple of fec k,n values,perform the test with all permutations
    test_fec_encode_and_decode_all_permutations(2,1,1024);
    test_fec_encode_and_decode_all_permutations(3,1,1024);
    test_fec_encode_and_decode_all_permutations(8,4,1024);
    test_fec_encode_and_decode_all_permutations(8,6,1024);
    test_fec_encode_and_decode_all_permutations(8,8,1024);
    test_fec_encode_and_decode_all_permutations(12,8,1024);
    std::cout<<"TEST_FEC passed\n";
}


void test_gf(){
    gf256_print_optimization_method();
    std::cout<<"Testing mul of 2 values\n";
    for(int i=0;i<256;i++){
        for(int j=0;j<256;j++){
            auto res1= gf256_mul(i,j);
            auto res2= gal_mul(i,j);
            assert(res1==res2);
        }
    }
    std::cout<<" - success.\n";

    std::cout<<"Testing gf256 mul operation (array)\n";
    for(int size=0;size<2048;size++){
        std::cout<<"x"<<std::flush;
        const auto source=FUCK::createRandomDataBuffer(size);
        std::vector<uint8_t> res1(size);
        std::vector<uint8_t> res2(size);
        for(int constant=0;constant<255;constant++){
            gal_mul_region(res1.data(),source.data(),constant,size);
            gf256_mul_optimized(res2.data(),source.data(),constant,size);
            FUCK::assertVectorsEqual(res1,res2);
        }
    }
    std::cout<<" - success.\n";

    std::cout<<"Testing gf256 madd operation (array)\n";
    for(int size=0;size<2048;size++){
        std::cout<<"x"<<std::flush;
        const auto source=FUCK::createRandomDataBuffer(size);
        const auto source2=FUCK::createRandomDataBuffer(size);
        for(int constant=0;constant<255;constant++){
            // other than mul, madd actually also reads from the dst array
            auto res1=source2;
            auto res2=source2;
            gal_madd_region(res1.data(),source.data(),constant,size);
            gf256_madd_optimized(res2.data(),source.data(),constant,size);
            FUCK::assertVectorsEqual(res1,res2);
        }
    }
    std::cout<<" - success.\n";
    std::cout<<"TEST_GF passed\n";
}

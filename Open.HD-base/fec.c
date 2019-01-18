/*#define PROFILE*/
/*
 * fec.c -- forward error correction based on Vandermonde matrices
 * 980624
 * (C) 1997-98 Luigi Rizzo (luigi@iet.unipi.it)
 * (C) 2001 Alain Knaff (alain@knaff.lu)
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
 * field elements. The code supports any value from 2 to 16
 * but fastest operation is achieved with 8 bit elements
 * This is the only parameter you may want to change.
 */
#define GF_BITS  8	/* code over GF(2**GF_BITS) - change to suit */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include "fec.h"

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

/*
 * You should not need to change anything beyond this point.
 * The first part of the file implements linear algebra in GF.
 *
 * gf is the type used to store an element of the Galois Field.
 * Must constain at least GF_BITS bits.
 *
 * Note: unsigned char will work up to GF(256) but int seems to run
 * faster on the Pentium. We use int whenever have to deal with an
 * index, since they are generally faster.
 */
/*
 * AK: Udpcast only uses GF_BITS=8. Remove other possibilities
 */
#if (GF_BITS != 8)
#error "GF_BITS must be 8"
#endif
typedef unsigned char gf;

#define	GF_SIZE ((1 << GF_BITS) - 1)	/* powers of \alpha */

/*
 * Primitive polynomials - see Lin & Costello, Appendix A,
 * and  Lee & Messerschmitt, p. 453.
 */
static char *allPp[] = {    /* GF_BITS	polynomial		*/
    NULL,		    /*  0	no code			*/
    NULL,		    /*  1	no code			*/
    "111",		    /*  2	1+x+x^2			*/
    "1101",		    /*  3	1+x+x^3			*/
    "11001",		    /*  4	1+x+x^4			*/
    "101001",		    /*  5	1+x^2+x^5		*/
    "1100001",		    /*  6	1+x+x^6			*/
    "10010001",		    /*  7	1 + x^3 + x^7		*/
    "101110001",	    /*  8	1+x^2+x^3+x^4+x^8	*/
    "1000100001",	    /*  9	1+x^4+x^9		*/
    "10010000001",	    /* 10	1+x^3+x^10		*/
    "101000000001",	    /* 11	1+x^2+x^11		*/
    "1100101000001",	    /* 12	1+x+x^4+x^6+x^12	*/
    "11011000000001",	    /* 13	1+x+x^3+x^4+x^13	*/
    "110000100010001",	    /* 14	1+x+x^6+x^10+x^14	*/
    "1100000000000001",	    /* 15	1+x+x^15		*/
    "11010000000010001"	    /* 16	1+x+x^3+x^12+x^16	*/
};


/*
 * To speed up computations, we have tables for logarithm, exponent
 * and inverse of a number. If GF_BITS <= 8, we use a table for
 * multiplication as well (it takes 64K, no big deal even on a PDA,
 * especially because it can be pre-initialized an put into a ROM!),
 * otherwhise we use a table of logarithms.
 * In any case the macro gf_mul(x,y) takes care of multiplications.
 */

static gf gf_exp[2*GF_SIZE];	/* index->poly form conversion table	*/
static int gf_log[GF_SIZE + 1];	/* Poly->index form conversion table	*/
static gf inverse[GF_SIZE+1];	/* inverse of field elem.		*/
				/* inv[\alpha**i]=\alpha**(GF_SIZE-i-1)	*/

/*
 * modnn(x) computes x % GF_SIZE, where GF_SIZE is 2**GF_BITS - 1,
 * without a slow divide.
 */
static inline gf
modnn(int x)
{
    while (x >= GF_SIZE) {
	x -= GF_SIZE;
	x = (x >> GF_BITS) + (x & GF_SIZE);
    }
    return x;
}

#define SWAP(a,b,t) {t tmp; tmp=a; a=b; b=tmp;}

/*
 * gf_mul(x,y) multiplies two numbers. If GF_BITS<=8, it is much
 * faster to use a multiplication table.
 *
 * USE_GF_MULC, GF_MULC0(c) and GF_ADDMULC(x) can be used when multiplying
 * many numbers by the same constant. In this case the first
 * call sets the constant, and others perform the multiplications.
 * A value related to the multiplication is held in a local variable
 * declared with USE_GF_MULC . See usage in addmul1().
 */
static gf gf_mul_table[(GF_SIZE + 1)*(GF_SIZE + 1)] 
#ifdef WINDOWS
__attribute__((aligned (16)))
#else
__attribute__((aligned (256)))
#endif
;

#define gf_mul(x,y) gf_mul_table[(x<<8)+y]

#define USE_GF_MULC register gf * __gf_mulc_
#define GF_MULC0(c) __gf_mulc_ = &gf_mul_table[(c)<<8]
#define GF_ADDMULC(dst, x) dst ^= __gf_mulc_[x]
#define GF_MULC(dst, x) dst = __gf_mulc_[x]

static void
init_mul_table(void)
{
    int i, j;
    for (i=0; i< GF_SIZE+1; i++)
	for (j=0; j< GF_SIZE+1; j++)
	    gf_mul_table[(i<<8)+j] = gf_exp[modnn(gf_log[i] + gf_log[j]) ] ;

    for (j=0; j< GF_SIZE+1; j++)
	gf_mul_table[j] = gf_mul_table[j<<8] = 0;
}

/*
 * Generate GF(2**m) from the irreducible polynomial p(X) in p[0]..p[m]
 * Lookup tables:
 *     index->polynomial form		gf_exp[] contains j= \alpha^i;
 *     polynomial form -> index form	gf_log[ j = \alpha^i ] = i
 * \alpha=x is the primitive element of GF(2^m)
 *
 * For efficiency, gf_exp[] has size 2*GF_SIZE, so that a simple
 * multiplication of two numbers can be resolved without calling modnn
 */



/*
 * initialize the data structures used for computations in GF.
 */
static void
generate_gf(void)
{
    int i;
    gf mask;
    char *Pp =  allPp[GF_BITS] ;

    mask = 1;	/* x ** 0 = 1 */
    gf_exp[GF_BITS] = 0; /* will be updated at the end of the 1st loop */
    /*
     * first, generate the (polynomial representation of) powers of \alpha,
     * which are stored in gf_exp[i] = \alpha ** i .
     * At the same time build gf_log[gf_exp[i]] = i .
     * The first GF_BITS powers are simply bits shifted to the left.
     */
    for (i = 0; i < GF_BITS; i++, mask <<= 1 ) {
	gf_exp[i] = mask;
	gf_log[gf_exp[i]] = i;
	/*
	 * If Pp[i] == 1 then \alpha ** i occurs in poly-repr
	 * gf_exp[GF_BITS] = \alpha ** GF_BITS
	 */
	if ( Pp[i] == '1' )
	    gf_exp[GF_BITS] ^= mask;
    }
    /*
     * now gf_exp[GF_BITS] = \alpha ** GF_BITS is complete, so can als
     * compute its inverse.
     */
    gf_log[gf_exp[GF_BITS]] = GF_BITS;
    /*
     * Poly-repr of \alpha ** (i+1) is given by poly-repr of
     * \alpha ** i shifted left one-bit and accounting for any
     * \alpha ** GF_BITS term that may occur when poly-repr of
     * \alpha ** i is shifted.
     */
    mask = 1 << (GF_BITS - 1 ) ;
    for (i = GF_BITS + 1; i < GF_SIZE; i++) {
	if (gf_exp[i - 1] >= mask)
	    gf_exp[i] = gf_exp[GF_BITS] ^ ((gf_exp[i - 1] ^ mask) << 1);
	else
	    gf_exp[i] = gf_exp[i - 1] << 1;
	gf_log[gf_exp[i]] = i;
    }
    /*
     * log(0) is not defined, so use a special value
     */
    gf_log[0] =	GF_SIZE ;
    /* set the extended gf_exp values for fast multiply */
    for (i = 0 ; i < GF_SIZE ; i++)
	gf_exp[i + GF_SIZE] = gf_exp[i] ;

    /*
     * again special cases. 0 has no inverse. This used to
     * be initialized to GF_SIZE, but it should make no difference
     * since noone is supposed to read from here.
     */
    inverse[0] = 0 ;
    inverse[1] = 1;
    for (i=2; i<=GF_SIZE; i++)
	inverse[i] = gf_exp[GF_SIZE-gf_log[i]];
}

/*
 * Various linear algebra operations that i use often.
 */

/*
 * addmul() computes dst[] = dst[] + c * src[]
 * This is used often, so better optimize it! Currently the loop is
 * unrolled 16 times, a good value for 486 and pentium-class machines.
 * The case c=0 is also optimized, whereas c=1 is not. These
 * calls are unfrequent in my typical apps so I did not bother.
 * 
 * Note that gcc on
 */
#if 0
#define addmul(dst, src, c, sz) \
    if (c != 0) addmul1(dst, src, c, sz)
#endif



#define UNROLL 16 /* 1, 4, 8, 16 */
static void
slow_addmul1(gf *dst1, gf *src1, gf c, int sz)
{
    USE_GF_MULC ;
    register gf *dst = dst1, *src = src1 ;
    gf *lim = &dst[sz - UNROLL + 1] ;

    GF_MULC0(c) ;

#if (UNROLL > 1) /* unrolling by 8/16 is quite effective on the pentium */
    for (; dst < lim ; dst += UNROLL, src += UNROLL ) {
	GF_ADDMULC( dst[0] , src[0] );
	GF_ADDMULC( dst[1] , src[1] );
	GF_ADDMULC( dst[2] , src[2] );
	GF_ADDMULC( dst[3] , src[3] );
#if (UNROLL > 4)
	GF_ADDMULC( dst[4] , src[4] );
	GF_ADDMULC( dst[5] , src[5] );
	GF_ADDMULC( dst[6] , src[6] );
	GF_ADDMULC( dst[7] , src[7] );
#endif
#if (UNROLL > 8)
	GF_ADDMULC( dst[8] , src[8] );
	GF_ADDMULC( dst[9] , src[9] );
	GF_ADDMULC( dst[10] , src[10] );
	GF_ADDMULC( dst[11] , src[11] );
	GF_ADDMULC( dst[12] , src[12] );
	GF_ADDMULC( dst[13] , src[13] );
	GF_ADDMULC( dst[14] , src[14] );
	GF_ADDMULC( dst[15] , src[15] );
#endif
    }
#endif
    lim += UNROLL - 1 ;
    for (; dst < lim; dst++, src++ )		/* final components */
	GF_ADDMULC( *dst , *src );
}

#if defined i386 && defined USE_ASSEMBLER

#define LOOPSIZE 8

static void
addmul1(gf *dst1, gf *src1, gf c, int sz)
{
    USE_GF_MULC ;

    GF_MULC0(c) ;

    if(((unsigned long)dst1 % LOOPSIZE) || 
       ((unsigned long)src1 % LOOPSIZE) || 
       (sz % LOOPSIZE)) {
	slow_addmul1(dst1, src1, c, sz);
	return;
    }

    asm volatile("xorl %%eax,%%eax;\n"
		 "	xorl %%edx,%%edx;\n"
		 ".align 32;\n"
		 "1:"
		 "	addl  $8, %%edi;\n"
		 
		 "	movb  (%%esi), %%al;\n"
		 "	movb 4(%%esi), %%dl;\n"
		 "	movb  (%%ebx,%%eax), %%al;\n"
		 "	movb  (%%ebx,%%edx), %%dl;\n"
		 "	xorb  %%al,  (%%edi);\n"
		 "	xorb  %%dl, 4(%%edi);\n"
		 
		 "	movb 1(%%esi), %%al;\n"
		 "	movb 5(%%esi), %%dl;\n"
		 "	movb  (%%ebx,%%eax), %%al;\n"
		 "	movb  (%%ebx,%%edx), %%dl;\n"
		 "	xorb  %%al, 1(%%edi);\n"
		 "	xorb  %%dl, 5(%%edi);\n"
		 
		 "	movb 2(%%esi), %%al;\n"
		 "	movb 6(%%esi), %%dl;\n"
		 "	movb  (%%ebx,%%eax), %%al;\n"
		 "	movb  (%%ebx,%%edx), %%dl;\n"
		 "	xorb  %%al, 2(%%edi);\n"
		 "	xorb  %%dl, 6(%%edi);\n"
		 
		 "	movb 3(%%esi), %%al;\n"
		 "	movb 7(%%esi), %%dl;\n"
		 "	addl  $8, %%esi;\n"
		 "	movb  (%%ebx,%%eax), %%al;\n"
		 "	movb  (%%ebx,%%edx), %%dl;\n"
		 "	xorb  %%al, 3(%%edi);\n"
		 "	xorb  %%dl, 7(%%edi);\n"
		 
		 "	cmpl  %%ecx, %%esi;\n"
		 "	jb 1b;"
		 : : 
		 
		 "b" (__gf_mulc_),
		 "D" (dst1-8),
		 "S" (src1),
		 "c" (sz+src1) :
		 "memory", "eax", "edx"
	);
}
#else
# define addmul1 slow_addmul1
#endif

static void addmul(gf *dst, gf *src, gf c, int sz) {
    // fprintf(stderr, "Dst=%p Src=%p, gf=%02x sz=%d\n", dst, src, c, sz);
    if (c != 0) addmul1(dst, src, c, sz);
}

/*
 * mul() computes dst[] = c * src[]
 * This is used often, so better optimize it! Currently the loop is
 * unrolled 16 times, a good value for 486 and pentium-class machines.
 * The case c=0 is also optimized, whereas c=1 is not. These
 * calls are unfrequent in my typical apps so I did not bother.
 * 
 * Note that gcc on
 */
#if 0
#define mul(dst, src, c, sz) \
    do { if (c != 0) mul1(dst, src, c, sz); else memset(dst, 0, sz); } while(0)
#endif

#define UNROLL 16 /* 1, 4, 8, 16 */
static void
slow_mul1(gf *dst1, gf *src1, gf c, int sz)
{
    USE_GF_MULC ;
    register gf *dst = dst1, *src = src1 ;
    gf *lim = &dst[sz - UNROLL + 1] ;

    GF_MULC0(c) ;

#if (UNROLL > 1) /* unrolling by 8/16 is quite effective on the pentium */
    for (; dst < lim ; dst += UNROLL, src += UNROLL ) {
	GF_MULC( dst[0] , src[0] );
	GF_MULC( dst[1] , src[1] );
	GF_MULC( dst[2] , src[2] );
	GF_MULC( dst[3] , src[3] );
#if (UNROLL > 4)
	GF_MULC( dst[4] , src[4] );
	GF_MULC( dst[5] , src[5] );
	GF_MULC( dst[6] , src[6] );
	GF_MULC( dst[7] , src[7] );
#endif
#if (UNROLL > 8)
	GF_MULC( dst[8] , src[8] );
	GF_MULC( dst[9] , src[9] );
	GF_MULC( dst[10] , src[10] );
	GF_MULC( dst[11] , src[11] );
	GF_MULC( dst[12] , src[12] );
	GF_MULC( dst[13] , src[13] );
	GF_MULC( dst[14] , src[14] );
	GF_MULC( dst[15] , src[15] );
#endif
    }
#endif
    lim += UNROLL - 1 ;
    for (; dst < lim; dst++, src++ )		/* final components */
	GF_MULC( *dst , *src );
}

#if defined i386 && defined USE_ASSEMBLER
static void
mul1(gf *dst1, gf *src1, gf c, int sz)
{
    USE_GF_MULC ;

    GF_MULC0(c) ;

    if(((unsigned long)dst1 % LOOPSIZE) || 
       ((unsigned long)src1 % LOOPSIZE) || 
       (sz % LOOPSIZE)) {
	slow_mul1(dst1, src1, c, sz);
	return;
    }

    asm volatile("pushl %%eax;\n"
		 "pushl %%edx;\n"
		 "xorl %%eax,%%eax;\n"
		 "	xorl %%edx,%%edx;\n"
		 "1:"
		 "	addl  $8, %%edi;\n"
		 
		 "	movb  (%%esi), %%al;\n"
		 "	movb 4(%%esi), %%dl;\n"
		 "	movb  (%%ebx,%%eax), %%al;\n"
		 "	movb  (%%ebx,%%edx), %%dl;\n"
		 "	movb  %%al,  (%%edi);\n"
		 "	movb  %%dl, 4(%%edi);\n"
		 
		 "	movb 1(%%esi), %%al;\n"
		 "	movb 5(%%esi), %%dl;\n"
		 "	movb  (%%ebx,%%eax), %%al;\n"
		 "	movb  (%%ebx,%%edx), %%dl;\n"
		 "	movb  %%al, 1(%%edi);\n"
		 "	movb  %%dl, 5(%%edi);\n"
		 
		 "	movb 2(%%esi), %%al;\n"
		 "	movb 6(%%esi), %%dl;\n"
		 "	movb  (%%ebx,%%eax), %%al;\n"
		 "	movb  (%%ebx,%%edx), %%dl;\n"
		 "	movb  %%al, 2(%%edi);\n"
		 "	movb  %%dl, 6(%%edi);\n"
		 
		 "	movb 3(%%esi), %%al;\n"
		 "	movb 7(%%esi), %%dl;\n"
		 "	addl  $8, %%esi;\n"
		 "	movb  (%%ebx,%%eax), %%al;\n"
		 "	movb  (%%ebx,%%edx), %%dl;\n"
		 "	movb  %%al, 3(%%edi);\n"
		 "	movb  %%dl, 7(%%edi);\n"
		 
		 "	cmpl  %%ecx, %%esi;\n"
		 "	jb 1b;\n"
		 "	popl %%edx;\n"
		 "	popl %%eax;"
		 : : 
		 
		 "b" (__gf_mulc_),
		 "D" (dst1-8),
		 "S" (src1),
		 "c" (sz+src1) :
		 "memory", "eax", "edx"
	);
}
#else
# define mul1 slow_mul1
#endif

static inline void mul(gf *dst, gf *src, gf c, int sz) {
    /*fprintf(stderr, "%p = %02x * %p\n", dst, c, src);*/
    if (c != 0) mul1(dst, src, c, sz); else memset(dst, 0, sz);
}

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
		c = inverse[ c ] ;
	    pivot_row[icol] = 1 ;
	    for (ix = 0 ; ix < k ; ix++ )
		pivot_row[ix] = gf_mul(c, pivot_row[ix] );
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
		    addmul(p, pivot_row, c, k );
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


static int fec_initialized = 0 ;

void fec_init(void)
{
    TICK(ticks[0]);
    generate_gf();
    TOCK(ticks[0]);
    DDB(fprintf(stderr, "generate_gf took %ldus\n", ticks[0]);)
	TICK(ticks[0]);
    init_mul_table();
    TOCK(ticks[0]);
    DDB(fprintf(stderr, "init_mul_table took %ldus\n", ticks[0]);)
	fec_initialized = 1 ;
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
		unsigned char **data_blocks,
		unsigned int nrDataBlocks,
		unsigned char **fec_blocks,
		unsigned int nrFecBlocks)

{
    unsigned int blockNo; /* loop for block counter */
    unsigned int row, col;

    assert(fec_initialized);    
    assert(nrDataBlocks <= 128);    
    assert(nrFecBlocks <= 128);

    if(!nrDataBlocks)
	return;

    for(row=0; row < nrFecBlocks; row++)
	mul(fec_blocks[row], data_blocks[0], inverse[128 ^ row], blockSize);
    
    for(col=129, blockNo=1; blockNo < nrDataBlocks; col++, blockNo ++) {
	for(row=0; row < nrFecBlocks; row++)
	    addmul(fec_blocks[row], data_blocks[blockNo],
		   inverse[row ^ col],
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
			  unsigned char **data_blocks,
			  unsigned int nr_data_blocks,
			  unsigned char **fec_blocks,
			  unsigned int *fec_block_nos,
			  unsigned int *erased_blocks,
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
	    unsigned char *src = data_blocks[col];
	    int j;
	    for(j=0; j < nr_fec_blocks; j++) {
		int blno = fec_block_nos[j];
		addmul(fec_blocks[j],src,inverse[blno^col^128],blockSize);
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
			   unsigned char **data_blocks,
			   unsigned char **fec_blocks,
			   unsigned int *fec_block_nos,
			   unsigned int *erased_blocks,
			   short nr_fec_blocks)
{
#ifdef PROFILE
    long long begin;
#endif
    /* construct matrix */
    int row;
    unsigned char matrix[nr_fec_blocks*nr_fec_blocks];
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
	    matrix[ptr] = inverse[irow ^ icol];
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
	unsigned char *target = data_blocks[erased_blocks[row]];
	mul(target,fec_blocks[0],matrix[ptr++],blockSize);
	for(col = 1; col < nr_fec_blocks;  col++,ptr++) {
	    addmul(target,fec_blocks[col],matrix[ptr],blockSize);
	}
    }
}

void fec_decode(unsigned int blockSize,

		unsigned char **data_blocks,
		unsigned int nr_data_blocks,
		unsigned char **fec_blocks,
		unsigned int *fec_block_nos,
		unsigned int *erased_blocks,
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


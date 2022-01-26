//
// Created by consti10 on 04.01.22.
//
// By including this file we get all the "(fast) galois field math" we need for our FEC implementation
// The mul and addmul methods are highly optimized for each architecture (see Readme.md)
// NOTE: Since the optimized methods only work on memory chuncks that are multiple of X,
// the slow method is still needed on them - but only performed on "a couple of bytes" so not much of an issue
// Also NOTE:
// I assume that every ARM platform has NEON support and every
// X86 platform has SSSE3 support

#ifndef WIFIBROADCAST_GF256_SIMPLE_INCLUDE_H
#define WIFIBROADCAST_GF256_SIMPLE_INCLUDE_H

// we always use the flat table - either as fallback or for chunks not of size X
#include "gf256_flat_table.h"
#include "alignment_check.h"

#ifdef __arm__
#define FEC_GF256_USE_ARM_NEON
#endif
#ifdef __x86_64__
#define FEC_GF256_USE_X86_SSSE3
#endif

//#define FEC_GF256_USE_X86_SSSE3

// include the optimized methods if enabled
#ifdef FEC_GF256_USE_ARM_NEON
#include "gf256_neon.h"
#endif

#ifdef FEC_GF256_USE_X86_SSSE3
#include "gf256_ssse3.h"
#endif

#include <iostream>
#include <cassert>


// computes dst[] = c * src[]
// where '+', '*' are gf256 operations
static void gf256_mul_optimized(uint8_t* dst,const uint8_t* src, gf c,const int sz){
#ifdef FEC_GF256_USE_X86_SSSE3
    const int sizeSlow = sz % 16;
    const int sizeFast = sz - sizeSlow;
    if(sizeFast>0){
        mulrc256_shuffle_ssse3(dst,src,c,sizeFast);
    }
    if(sizeSlow>0){
        mulrc256_flat_table(&dst[sizeFast],&src[sizeFast],c,sizeSlow);
    }
#elif defined(FEC_GF256_USE_ARM_NEON)
    const int sizeSlow = sz % 8;
    const int sizeFast = sz - sizeSlow;
    if(sizeFast>0){
        mulrc256_shuffle_neon_64(dst,src,c,sizeFast);
    }
    if(sizeSlow>0){
        mulrc256_flat_table(&dst[sizeFast],&src[sizeFast],c,sizeSlow);
    }
#else
    mulrc256_flat_table(dst,src,c,sz);
#endif
}

// computes dst[] = dst[] + c * src[]
// where '+', '*' are gf256 operations
static void gf256_madd_optimized(uint8_t* dst,const uint8_t* src, gf c,const int sz){
#ifdef FEC_GF256_USE_X86_SSSE3
    const int sizeSlow = sz % 16;
    const int sizeFast = sz - sizeSlow;
    if(sizeFast>0){
        maddrc256_shuffle_ssse3(dst,src,c,sizeFast);
    }
    if(sizeSlow>0){
        maddrc256_flat_table(&dst[sizeFast],&src[sizeFast],c,sizeSlow);
    }
    //maddrc256_flat_table(dst,src,c,sz);
#elif defined(FEC_GF256_USE_ARM_NEON)
    const int sizeSlow = sz % 8;
    const int sizeFast = sz - sizeSlow;
    if(sizeFast>0){
        maddrc256_shuffle_neon_64(dst,src,c,sizeFast);
    }
    if(sizeSlow>0){
        maddrc256_flat_table(&dst[sizeFast],&src[sizeFast],c,sizeSlow);
    }
#else
    maddrc256_flat_table(dst,src,c,sz);
#endif
}

static const uint8_t inverses[MOEPGF256_SIZE] = MOEPGF256_INV_TABLE;

// for the inverse of a number we don't have a highly optimized method
// since it is never done on big chunks of memory anyways
static uint8_t gf256_inverse(uint8_t value){
    return inverses[value];
}

// and sometimes the FEC code needs to just multiply two uint8_t values (not a memory region)
static uint8_t gf256_mul(uint8_t x,uint8_t y){
    uint8_t ret;
    mulrc256_flat_table(&ret,&x,y,1);
    return ret;
}

static void gf256_print_optimization_method(){
#ifdef FEC_GF256_USE_X86_SSSE3
    std::cout<<"Using X86_SSSE3 optimization\n";
#elif defined(FEC_GF256_USE_ARM_NEON)
    std::cout<<"Using ARM_NEON optimization\n";
#else
    std::cout<<"No optimization, using flat_table as fallback\n";
#endif
}

#endif //WIFIBROADCAST_GF256_SIMPLE_INCLUDE_H

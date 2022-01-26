//
// Created by consti10 on 02.01.22.
//
#ifndef LIBMOEPGF_GF256_NEON_H
#define LIBMOEPGF_GF256_NEON_H

#include <arm_neon.h>
#include "gf256tables285.h"
#include <stdint.h>

#include <iostream>

// Fastest if NEON is supported
// Regrading alignment: https://developer.arm.com/documentation/ddi0344/f/Cihejdic
// I think neon by default doesn't care about alignment, only if the alignment is explicitly specified it needs to match

static const uint8_t tl[MOEPGF256_SIZE][16] = MOEPGF256_SHUFFLE_LOW_TABLE;
static const uint8_t th[MOEPGF256_SIZE][16] = MOEPGF256_SHUFFLE_HIGH_TABLE;

void
xorr_neon_64(uint8_t *region1, const uint8_t *region2, size_t length)
{
    //std::cout<<"Xx neon"<<length<<" "<<(int)region1[0]<<":"<<(int)region2[0]<<"\n";
    assert(length % 8 ==0);
    uint8_t *end;
    register uint8x8_t in,out;

    for (end=region1+length; region1<end; region1+=8, region2+=8) {
        in  = vld1_u8((const uint8_t *)region1);
        out = vld1_u8((const uint8_t *)region2);
        out = veor_u8(in, out);
        vst1_u8((uint8_t *)region1, out);
    }
    /*uint8_t *end;
    register uint64x1_t in, out;

    for (end=region1+length; region1<end; region1+=8, region2+=8) {
        in  = vld1_u64((const uint64_t *)region2);
        out = vld1_u64((const uint64_t *)region1);
        out = veor_u64(in, out);
        vst1_u64((uint64_t  *)region1, out);
    }*/
    //std::cout<<"Yi neon\n";
}

// Consti10 NOTE: only works when size % 8==0
static void
maddrc256_shuffle_neon_64(uint8_t *region1, const uint8_t *region2,
                          uint8_t constant, size_t length)
{
    assert(length % 8 ==0);
    uint8_t *end;
    register uint8x8x2_t t1, t2;
    register uint8x8_t m1, m2, in1, in2, out, l, h;

    if (constant == 0)
        return;

    if (constant == 1) {
        xorr_neon_64(region1, region2, length);
        return;
    }

    t1 = vld2_u8((const uint8_t *)tl[constant]);
    t2 = vld2_u8((const uint8_t *)th[constant]);
    m1 = vdup_n_u8(0x0f);
    m2 = vdup_n_u8(0xf0);

    for (end=region1+length; region1<end; region1+=8, region2+=8) {
        in2 = vld1_u8((const uint8_t *)region2);
        in1 = vld1_u8((const uint8_t *)region1);
        l = vand_u8(in2, m1);
        l = vtbl2_u8(t1, l);
        h = vand_u8(in2, m2);
        h = vshr_n_u8(h, 4);
        h = vtbl2_u8(t2, h);
        out = veor_u8(h, l);
        out = veor_u8(out, in1);
        vst1_u8(region1, out);
    }
}


// Consti10 - modified to write output into a different memory region than input
void
mulrc256_shuffle_neon_64(uint8_t *region1,const uint8_t* region2, uint8_t constant, size_t length)
{
    assert(length % 8 ==0);
    uint8_t *end;
    register uint8x8x2_t t1, t2;
    register uint8x8_t m1, m2, in, out, l, h;

    if (constant == 0) {
        memset(region1, 0, length);
        return;
    }

    if (constant == 1){
        memcpy(region1,region2,length);
        return;
    }

    t1 = vld2_u8((const uint8_t *)tl[constant]);
    t2 = vld2_u8((const uint8_t *)th[constant]);
    m1 = vdup_n_u8(0x0f);
    m2 = vdup_n_u8(0xf0);

    for (end=region1+length; region1<end; region1+=8,region2+=8) {
        in = vld1_u8((const uint8_t *)region2);
        l = vand_u8(in, m1);
        l = vtbl2_u8(t1, l);
        h = vand_u8(in, m2);
        h = vshr_n_u8(h, 4);
        h = vtbl2_u8(t2, h);
        out = veor_u8(h, l);
        vst1_u8(region1, out);
    }
}

#endif //LIBMOEPGF_GF256_NEON_H
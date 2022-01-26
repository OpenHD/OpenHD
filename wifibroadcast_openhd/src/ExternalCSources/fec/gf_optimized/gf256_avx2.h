//
// Created by consti10 on 04.01.22.
//

#ifndef LIBMOEPGF_GF256_AVX2_H
#define LIBMOEPGF_GF256_AVX2_H

#include <immintrin.h>
#include "gf256tables285.h"
#include <stdint.h>
#include <iostream>

// fastest option for x86 if AVX2 is supported
// NOTE - I ended up not using the AVX2 code for the following reasons:
// 1) since working on 32 bytes, the alignment issue is even more apparent
// 2) ssse3 is already "fast enough" and supported on pretty much any platform (other than AVX2)
// If you want to, add and test the code here ;)

static const uint8_t tl[MOEPGF256_SIZE][16] = MOEPGF256_SHUFFLE_LOW_TABLE;
static const uint8_t th[MOEPGF256_SIZE][16] = MOEPGF256_SHUFFLE_HIGH_TABLE;

static inline bool is_aligned(const void * pointer, size_t byte_count)
{ return (uintptr_t)pointer % byte_count == 0; }

void
xorr_avx2(uint8_t *region1, const uint8_t *region2, size_t length)
{
    assert(length % 32 ==0);
    std::cout<<"xorr_avx2\n";
    assert(is_aligned(region1,32));
    assert(is_aligned(region2,32));
    //std::cout<<""<<(int)region1[length]<<":"<<(int)region2[length]<<"\n";

    uint8_t *end;
    register __m256i in, out;

    for (end=region1+length; region1<end; region1+=32, region2+=32) {
        std::cout<<"XXx\n";
        /*in  = _mm256_load_si256((const __m256i*)region2);
        out = _mm256_load_si256((const __m256i*)region1);
        out = _mm256_xor_si256(in, out);
        _mm256_store_si256((__m256i *)region1, out);*/
        in  = _mm256_loadu_si256((const __m256i*)region2);
        out = _mm256_loadu_si256((const __m256i*)region1);
        out = _mm256_xor_si256(in, out);
        _mm256_storeu_si256((__m256i *)region1, out);
        std::cout<<"YY\n";
    }
}

void
maddrc256_shuffle_avx2(uint8_t *region1, const uint8_t *region2,
                       uint8_t constant, size_t length)
{
    assert(length % 32 ==0);
    //std::cout<<"maddrc256_shuffle_avx2:"<<length<<" constant:"<<(int)constant<<" "<<(int)region1[0]<<":"<<(int)region2[0]<<"\n";
    //assert(is_aligned(region1,32));
    //assert(is_aligned(region2,32));
    uint8_t *end;
    register __m256i t1, t2, m1, m2, in1, in2, out, l, h;
    register __m128i bc;

    if (constant == 0)
        return;

    if (constant == 1) {
        xorr_avx2(region1, region2, length);
        return;
    }

    bc = _mm_load_si128((const __m128i *)tl[constant]);
    t1 = __builtin_ia32_vbroadcastsi256(bc);
    bc = _mm_load_si128((const __m128i* )th[constant]);
    t2 = __builtin_ia32_vbroadcastsi256(bc);
    m1 = _mm256_set1_epi8(0x0f);
    m2 = _mm256_set1_epi8(0xf0);

    for (end=region1+length; region1<end; region1+=32, region2+=32) {
        in2 = _mm256_load_si256((const __m256i *)region2);
        in1 = _mm256_load_si256((const __m256i *)region1);
        l = _mm256_and_si256(in2, m1);
        l = _mm256_shuffle_epi8(t1, l);
        h = _mm256_and_si256(in2, m2);
        h = _mm256_srli_epi64(h, 4);
        h = _mm256_shuffle_epi8(t2, h);
        out = _mm256_xor_si256(h, l);
        out = _mm256_xor_si256(out, in1);
        _mm256_store_si256((__m256i *)region1, out);
    }
}

void
mulrc256_shuffle_avx2(uint8_t *region1,const uint8_t * region2,uint8_t constant, size_t length)
{
    assert(length % 32 ==0);
    //assert(is_aligned(region1,32));
    //assert(is_aligned(region2,32));
    uint8_t *end;
    register __m256i t1, t2, m1, m2, in, out, l, h;
    register __m128i bc;

    if (constant == 0) {
        memset(region1, 0, length);
        return;
    }

    if (constant == 1)
        return;

    bc = _mm_load_si128((const __m128i *)tl[constant]);
    t1 = __builtin_ia32_vbroadcastsi256(bc);
    bc = _mm_load_si128((const __m128i *)th[constant]);
    t2 = __builtin_ia32_vbroadcastsi256(bc);
    m1 = _mm256_set1_epi8(0x0f);
    m2 = _mm256_set1_epi8(0xf0);

    for (end=region1+length; region1<end; region1+=32,region2+=32) {
        in = _mm256_load_si256((const __m256i *)region2);
        l = _mm256_and_si256(in, m1);
        l = _mm256_shuffle_epi8(t1, l);
        h = _mm256_and_si256(in, m2);
        h = _mm256_srli_epi64(h, 4);
        h = _mm256_shuffle_epi8(t2, h);
        out = _mm256_xor_si256(h, l);
        _mm256_store_si256((__m256i *)region1, out);
    }
}



#endif //LIBMOEPGF_GF256_AVX2_H

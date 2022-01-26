//
// Created by consti10 on 03.01.22.
//

#ifndef LIBMOEPGF_GF256_FLAT_TABLE_H
#define LIBMOEPGF_GF256_FLAT_TABLE_H

#include "gf256tables285.h"
#include <stdint.h>

// Slower, but compiles on any hardware

static const uint8_t mult[MOEPGF256_SIZE][MOEPGF256_SIZE] = MOEPGF256_MUL_TABLE;

static void
xorr_scalar(uint8_t *region1, const uint8_t *region2, size_t length)
{
    for(; length; region1++, region2++, length--)
        *region1 ^= *region2;
}

static void
maddrc256_flat_table(uint8_t *region1, const uint8_t *region2,
                     uint8_t constant, size_t length)
{
    if (constant == 0)
        return;

    if (constant == 1) {
        xorr_scalar(region1, region2, length);
        return ;
    }

    for (; length; region1++, region2++, length--) {
    	*region1 ^= mult[constant][*region2];
    }

}

static void
mulrc256_flat_table(uint8_t *region1, const uint8_t *region2,
                     uint8_t constant, size_t length)
{
    if (constant == 0)
        memset(region1, 0, length);

    if (constant == 1) {
        memcpy(region1,region2,length);
        return;
    }

    for (; length; region1++, region2++, length--) {
        *region1 = mult[constant][*region2];
    }
}




#endif //LIBMOEPGF_GF256_FLAT_TABLE_H

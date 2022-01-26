//
// Created by consti10 on 02.01.22.
//

#ifndef WIFIBROADCAST_GAL_MATH_REFERENCE_H
#define WIFIBROADCAST_GAL_MATH_REFERENCE_H

// from https://gist.github.com/meagtan/dc1adff8d84bb895891d8fd027ec9d8c
// This code exists to validate the (optimized) gf256 math operations at run time

typedef unsigned char gal8; /* Galois field of order 2^8 */

const gal8 min_poly  = 0b11101,     /* Minimal polynomial x^8 + x^4 + x^3 + x^2 + 1 */
generator = 0b10;        /* Generator of Galois field */

/* Add two elements of GF(2^8) */
gal8 gal_add(gal8 a, gal8 b)
{
    return a ^ b;
}

/* Multiply two elements of GF(2^8) */
gal8 gal_mul(gal8 a, gal8 b)
{
    gal8 res = 0;
    for (; b; b >>= 1) {
        if (b & 1)
            res ^= a;
        if (a & 0x80)
            a = (a << 1) ^ min_poly;
        else
            a <<= 1;
    }
    return res;
}

// Consti10
// multiply and add 3 elements of GF(2^8)
// return x + a*b
gal8 gal_madd(gal8 x,gal8 a,gal8 b){
    return gal_add(x, gal_mul(a,b));
}

// same as above, but for memory regions ( to test optimized versions)
static void
gal_mul_region(gf *dst,const gf *src, gf c,const int sz){
    for(int i=0;i<sz;i++){
        dst[i]=gal_mul(src[i],c);
    }
}
static void
gal_madd_region(gf *dst,const gf *src, gf c,const int sz){
    for(int i=0;i<sz;i++){
        dst[i]=gal_madd(dst[i],src[i],c);
    }
}


#endif //WIFIBROADCAST_GAL_MATH_REFERENCE_H

//
// Created by consti10 on 08.01.22.
//

#ifndef WIFIBROADCAST_ALIGNMENT_CHECK_H
#define WIFIBROADCAST_ALIGNMENT_CHECK_H

// In the beginning I had some problems with the optimized methods regarding the data alignment
// These issues are fixed now, though. The file is therefore obsolete.

#include <iostream>

static inline bool is_aligned(const void * pointer, size_t byte_count)
{ return (uintptr_t)pointer % byte_count == 0; }

static inline bool are_aligned(const void * pointer1,const void* pointer2,size_t byte_count)
{ return is_aligned(pointer1,byte_count) && is_aligned(pointer2,byte_count);}


// Not guaranteed that a value exists that has the proper alignment for both input arrays -
// well I was able to fix it otherwise
static inline int find_alignment(const uint8_t * pointer1,const uint8_t * pointer2, size_t byte_count){
    int ret=0;
    while (! ( is_aligned(&pointer1[ret],byte_count) && is_aligned(&pointer2[ret],byte_count) ) ) {
        ret+=8;
    }
    if(ret!=0){
        std::cout<<"Alignment okay after "<<ret<<"bytes\n";
    }
    return ret;
}

#endif //WIFIBROADCAST_ALIGNMENT_CHECK_H

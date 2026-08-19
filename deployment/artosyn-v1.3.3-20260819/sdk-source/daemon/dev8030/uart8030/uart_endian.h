#ifndef __UART_ENDIAN__
#define __UART_ENDIAN__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__linux__)
#include <endian.h>
#endif

static inline bool is_local_little_edian(void)
{
    uint32_t val = 0xaabbccdd;
    uint8_t *ptr = (uint8_t *)&val;

    if(ptr[0] == 0xdd)
        return true;
    else if(ptr[0] == 0xaa)
        return false;

    return false;
}

static inline uint16_t swap16(uint16_t data)
{
    return ((data<<8)&0xff00) | ((data>>8)&0xff);
}

static inline uint32_t swap32(uint32_t data)
{
    return ((data>>24)&0xff) | // move byte 3 to byte 0
        ((data<<8)&0xff0000) | // move byte 1 to byte 2
        ((data>>8)&0xff00) | // move byte 2 to byte 1
        ((data<<24)&0xff000000); // byte 0 to byte 3
}

#ifndef htobe16
static inline uint16_t htobe16(uint16_t data)
{
    if (is_local_little_edian()) {
        return swap16(data);
    }
    else {
        return data;
    }
}
#endif

#ifndef htole16
static inline uint16_t htole16(uint16_t data)
{
    if (is_local_little_edian()) {
        return data;
    }
    else {
        return swap16(data);
    }
}
#endif

#ifndef htobe32
static inline uint32_t htobe32(uint32_t data)
{
    if (is_local_little_edian()) {
        return swap32(data);
    }
    else {
        return data;
    }
}
#endif

#ifndef htole32
static inline uint32_t htole32(uint32_t data)
{
    if (is_local_little_edian()) {
        return data;
    }
    else {
        return swap32(data);
    }
}
#endif

#ifndef be16toh
static inline uint16_t be16toh(uint16_t data)
{
    if (is_local_little_edian()) {
        return swap16(data);
    }
    else {
        return data;
    }
}
#endif

#ifndef le16toh
static inline uint16_t le16toh(uint16_t data)
{
    if (is_local_little_edian()) {
        return data;
    }
    else {
        return swap16(data);
    }
}
#endif

#ifndef be32toh
static inline uint32_t be32toh(uint32_t data)
{
    if (is_local_little_edian()) {
        return swap32(data);
    }
    else {
        return data;
    }
}
#endif

#ifndef le32toh
static inline uint32_t le32toh(uint32_t data)
{
    if (is_local_little_edian()) {
        return data;
    }
    else {
        return swap32(data);
    }
}
#endif

#ifdef __cplusplus
}
#endif

#endif

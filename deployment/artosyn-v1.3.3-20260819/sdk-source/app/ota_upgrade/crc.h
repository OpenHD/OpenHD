#ifndef CRC_H
#define CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t getcrc32_update(uint32_t init_vect, uint8_t *buffer, uint32_t len);
uint32_t getcrc32(uint8_t *buffer, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* CRC_H */

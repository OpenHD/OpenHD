#ifndef __SESSION_SOCKET_DATAGRAM_EXT_H__
#define __SESSION_SOCKET_DATAGRAM_EXT_H__
#include <stdint.h>
#include "usbpack.h"

int make_datagram_pack2buff(unsigned char* buff, uint32_t len, usbpack* pack);
int unpack_datagram_pack(unsigned char* buff, uint32_t bufflen, int* usedlen, usbpack* pack);
int ss_head_cnt(void);
#endif

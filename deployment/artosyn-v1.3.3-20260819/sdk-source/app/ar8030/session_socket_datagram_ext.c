#include "session_socket_datagram_ext.h"
#include <string.h>

static const uint32_t fixpackbase = 1 + 4 + 1 + 1;

int ss_head_cnt(void)
{
    return fixpackbase - 1;
}

static uint8_t xor_check(unsigned char* buff, int len)
{
    uint8_t xor = 0xff;
    for (int i = 0; i < len; i++) {
        xor ^= buff[i];
    }
    return xor;
}

int make_datagram_pack2buff(unsigned char* buff, uint32_t len, usbpack* pack)
{
    uint32_t maxlen = fixpackbase + pack->datalen;

    if (len < maxlen) {
        return -1;
    }

    int offset = 0;

    buff[offset++] = 0xab;

    buff[offset++] = pack->datalen & 0xff;
    buff[offset++] = (pack->datalen >> 8) & 0xff;
    buff[offset++] = (pack->datalen >> 16) & 0xff;
    buff[offset++] = (pack->datalen >> 24) & 0xff;

    uint8_t orchk  = xor_check(buff, offset);
    buff[offset++] = orchk;

    memcpy(buff + offset, pack->datapack, pack->datalen);
    offset += pack->datalen;

    buff[offset++] = 0xbc;

    return maxlen;
}


int unpack_datagram_pack(unsigned char* buff, uint32_t bufflen, int* usedlen, usbpack* pack)
{
    if (!buff || bufflen < fixpackbase || !pack) {
        return -1;
    }

    for (uint32_t offset = 0; offset <= bufflen - fixpackbase; offset++) {
        if (buff[offset + 0] == 0xab) {
            // pack len
            uint32_t datalen = 0;
            datalen |= buff[offset + 1] << 0;
            datalen |= buff[offset + 2] << 8;
            datalen |= buff[offset + 3] << 16;
            datalen |= buff[offset + 4] << 24;

            uint32_t maxlen = fixpackbase + datalen;

            // chk len
            if (offset + maxlen > bufflen) {
                continue;
            }

            // chk tail
            if (buff[offset + maxlen - 1] != 0xbc) {
                continue;
            }

            // chk len
            uint8_t cal_xor = xor_check(buff + offset, fixpackbase - 2);

            if (cal_xor != buff[offset + fixpackbase - 2]) {
                continue;
            }

            if (usedlen) {
                *usedlen = offset + maxlen;
            }

            pack->datalen = datalen;
            // head + datalen

            if (pack->datalen) {
                pack->datapack = buff + offset + fixpackbase - 1;
            } else {
                pack->datapack = 0;
            }

            return 0;
        }
    }
    return -2;
}

#include "usbpack.h"
#include <string.h>

/**
 * @brief usbpack中的固定长度
 *
 */
static const uint32_t data_offset = 1 + 4 + 4 + 4 + 4 + 1;
static const uint32_t fixpackbase = 1 + 4 + 4 + 4 + 4 + 1 + 1;

int get_fix_usblen(void)
{
    return fixpackbase;
}

int get_usb_data_offset(void)
{
    return data_offset;
}

static uint8_t xor_check(unsigned char* buff, int len)
{
    uint8_t xor = 0xff;
    for (int i = 0; i < len; i++) {
        xor ^= buff[i];
    }
    return xor;
}

int make_usbpack2buff(unsigned char* buff, uint32_t len, usbpack* pack)
{
    uint32_t maxlen = fixpackbase + pack->datalen;

    if (len < maxlen) {
        return -1;
    }

    int offset = 0;

    buff[offset++] = 0xaa;

    buff[offset++] = pack->datalen & 0xff;
    buff[offset++] = (pack->datalen >> 8) & 0xff;
    buff[offset++] = (pack->datalen >> 16) & 0xff;
    buff[offset++] = (pack->datalen >> 24) & 0xff;

    buff[offset++] = pack->reqid >> 24;
    buff[offset++] = pack->reqid >> 16;
    buff[offset++] = pack->reqid >> 8;
    buff[offset++] = pack->reqid >> 0;

    buff[offset++] = pack->msgid >> 24;
    buff[offset++] = pack->msgid >> 16;
    buff[offset++] = pack->msgid >> 8;
    buff[offset++] = pack->msgid >> 0;

    buff[offset++] = pack->sta >> 24;
    buff[offset++] = pack->sta >> 16;
    buff[offset++] = pack->sta >> 8;
    buff[offset++] = pack->sta >> 0;

    uint8_t orchk  = xor_check(buff, offset);
    buff[offset++] = orchk;

    memcpy(buff + offset, pack->datapack, pack->datalen);
    offset += pack->datalen;

    buff[offset++] = 0xbb;

    return maxlen;
}

int make_usbpack2buff_head_only(unsigned char* buff, uint32_t len, usbpack* pack)
{
    uint32_t maxlen = fixpackbase + pack->datalen;

    if (len < maxlen) {
        return -1;
    }

    int offset = 0;

    buff[offset++] = 0xaa;

    buff[offset++] = pack->datalen & 0xff;
    buff[offset++] = (pack->datalen >> 8) & 0xff;
    buff[offset++] = (pack->datalen >> 16) & 0xff;
    buff[offset++] = (pack->datalen >> 24) & 0xff;

    buff[offset++] = pack->reqid >> 24;
    buff[offset++] = pack->reqid >> 16;
    buff[offset++] = pack->reqid >> 8;
    buff[offset++] = pack->reqid >> 0;

    buff[offset++] = pack->msgid >> 24;
    buff[offset++] = pack->msgid >> 16;
    buff[offset++] = pack->msgid >> 8;
    buff[offset++] = pack->msgid >> 0;

    buff[offset++] = pack->sta >> 24;
    buff[offset++] = pack->sta >> 16;
    buff[offset++] = pack->sta >> 8;
    buff[offset++] = pack->sta >> 0;

    uint8_t orchk  = xor_check(buff, offset);
    buff[offset++] = orchk;

    offset += pack->datalen;

    buff[offset++] = 0xbb;

    return maxlen;
}

/**
 * @brief 从二进制中找出usbpack
 *
 * @param buff
 * @param bufflen
 * @param usedlen 发现到的usbpack最后一个字节的位置
 * @param pack
 * @return int
 */
int unpack_usb_pack(unsigned char* buff, uint32_t bufflen, int* usedlen, usbpack* pack)
{
    if (!buff || bufflen < fixpackbase || !pack) {
        return -1;
    }

    for (uint32_t offset = 0; offset <= bufflen - fixpackbase; offset++) {
        if (buff[offset + 0] == 0xaa) {
            // pack len
            uint32_t datalen = 0;
            datalen |= buff[offset + 1] << 0;
            datalen |= buff[offset + 2] << 8;
            datalen |= buff[offset + 3] << 16;
            datalen |= buff[offset + 4] << 24;

            uint32_t maxlen = fixpackbase + datalen;

            // chk range
            if (datalen > (0xFFFFFFFF - fixpackbase)) {
                continue;
            }

            // chk len
            if (offset + maxlen - 1 >= bufflen) {
                continue;
            }

            // chk tail
            if (buff[offset + maxlen - 1] != 0xbb) {
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
            int reqoffset = offset + 5;

            pack->reqid = buff[reqoffset + 0] << 24;
            pack->reqid |= buff[reqoffset + 1] << 16;
            pack->reqid |= buff[reqoffset + 2] << 8;
            pack->reqid |= buff[reqoffset + 3] << 0;

            pack->msgid = buff[reqoffset + 4] << 24;
            pack->msgid |= buff[reqoffset + 5] << 16;
            pack->msgid |= buff[reqoffset + 6] << 8;
            pack->msgid |= buff[reqoffset + 7] << 0;

            pack->sta = buff[reqoffset + 8] << 24;
            pack->sta |= buff[reqoffset + 9] << 16;
            pack->sta |= buff[reqoffset + 10] << 8;
            pack->sta |= buff[reqoffset + 11] << 0;

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

#ifndef __BB_USB_PACK_H__
#define __BB_USB_PACK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct usbpack {
    union {
        struct {
            uint32_t subcmdid : 24;
            uint32_t domainid : 8;
        };
        uint32_t reqid;
    };
    uint32_t datalen;
    uint32_t msgid;
    int32_t  sta;
    union {
        uint8_t* datapack; /// 注意这个有可能是不对齐的
        void*    data_v;
    };
} usbpack;

int make_usbpack2buff(unsigned char* buff, uint32_t len, usbpack* pack);
int unpack_usb_pack(unsigned char* buff, uint32_t bufflen, int* usedlen, usbpack* pack);
int make_usbpack2buff_head_only(unsigned char* buff, uint32_t len, usbpack* pack);
int get_fix_usblen(void);
int get_usb_data_offset(void);
#ifdef __cplusplus
}
#endif

#endif

#ifndef __USB_DEV_H__
#define __USB_DEV_H__

#include "dev8030.h"
#include <stddef.h>
#include <stdint.h>

#define ARTO_RTOS_VID (0x1d6b)
#define ARTO_RTOS_PID (0x8030)

typedef struct usb8030_info   usb8030_info;
typedef struct libusb_context libusb_context;

typedef struct usbrpc_ctrl usbrpc_ctrl;
typedef struct rpc_info    rpc_info;


#endif

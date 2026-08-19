#ifndef __COM_LOG_H__
#define __COM_LOG_H__
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif
#define DAEMON_LOG_PATH "daemon_log"
typedef enum {
    COM_LOG_DEBUG = 0,
    COM_LOG_INFO,
    COM_LOG_WARN,
    COM_LOG_ERROR,
    COM_LOG_MAX,
} com_log_level;

typedef enum {
    COM_LOG_LEVLE = -1,

    COM_NET = 0,
    COM_INIT,

    COM_IOCTL_COM,

    COM_SOCKET_COM,
    COM_SOCKET_DATA,

    COM_USB_COM,
    COM_USB_DATA,

    COM_UART_COM,

    COM_SDIO_COM,

    COM_HOT_PLUG_COM,
    COM_HOT_PLUG_DAT,

    COM_CALLBACK_COM,
    COM_CALLBACK_DAT,

    COM_BASE_COM,

    COM_CMDLINE,

    COM_DRV_COM,

    COM_MODULE_MAX,
} com_log_module;

int com_vprintf(const char* fun, com_log_module module, const char* fmt, ...);

void com_log_init(char* file);

int com_log_get_level(char* name);

void com_log_set_level(char* name, int level);

#define com_log(module, fmt, ...)                                                                                      \
    do {                                                                                                               \
        com_vprintf(__func__, module, fmt, ##__VA_ARGS__);                                                             \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif

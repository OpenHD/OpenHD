#include "com_log.h"
#include "bg_proc.h"
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

#if defined(__ANDROID__)
#include <android/log.h>
#endif

typedef struct {
    com_log_module mod;
    const char*    name_prefix;
    com_log_level  level;
} module_info;

static FILE*    log_fp = NULL;
static bg_proc* bgptr  = NULL;

static module_info _infos[] = {
    { COM_LOG_LEVLE,    "print_level", COM_LOG_DEBUG },

    { COM_NET,          "net",         COM_LOG_INFO  },
    { COM_INIT,         "init",        COM_LOG_INFO  },

    { COM_IOCTL_COM,    "ioctl",       COM_LOG_INFO  },

    { COM_SOCKET_COM,   "sock-com",    COM_LOG_INFO  },
    { COM_SOCKET_DATA,  "sock-dat",    COM_LOG_DEBUG },

    { COM_USB_COM,      "USB-com",     COM_LOG_WARN  },
    { COM_USB_DATA,     "USB-dat",     COM_LOG_DEBUG },

    { COM_UART_COM,     "UART-com",    COM_LOG_WARN  },

    { COM_SDIO_COM,     "sdio-com",    COM_LOG_WARN  },

    { COM_HOT_PLUG_COM, "hotplug-com", COM_LOG_INFO  },
    { COM_HOT_PLUG_DAT, "hotplug-dat", COM_LOG_DEBUG },

    { COM_CALLBACK_COM, "cb-com",      COM_LOG_INFO  },
    { COM_CALLBACK_DAT, "cb-dat",      COM_LOG_DEBUG },

    { COM_BASE_COM,     "base-dat",    COM_LOG_INFO  },
    { COM_CMDLINE,      "cmdline",     COM_LOG_INFO  },
    { COM_DRV_COM,      "drv-com",     COM_LOG_INFO  },
};

static module_info* find_module_info(const char* name)
{
    int sz = sizeof(_infos) / sizeof(_infos[0]);

    for (int i = 0; i < sz; i++) {
        module_info* pinfo = &_infos[i];
        if (!strcmp(name, pinfo->name_prefix)) {
            return pinfo;
        }
    }
    return NULL;
}

void com_log_set_level(char* name, int level)
{
    module_info* pinfo = find_module_info(name);
    if (pinfo) {
        pinfo->level = level;
    }
}

int com_log_get_level(char* name)
{
    module_info* pinfo = find_module_info(name);
    if (pinfo) {
        return pinfo->level;
    }
    return -1;
}

static int _com_vprintf(const char* fun, const char* module, const char* level, const char* fmt, va_list va)
{
#define MAX_BUF (4096)
    char buff[MAX_BUF];

    struct timespec spec;
#ifdef WIN32
    timespec_get(&spec, TIME_UTC);
#else
    clock_gettime(CLOCK_REALTIME, &spec);
#endif

    int len = sprintf(buff,
                      "[%d.%06d] [%p] [%s] [%s] %s:",
                      (int)spec.tv_sec,
                      (int)(spec.tv_nsec / 1000),
#ifdef WIN32
                      pthread_self().p,
#else
                      (void*)pthread_self(),
#endif
                      level,
                      module,
                      fun);

    len += vsnprintf(buff + len, MAX_BUF - len, fmt, va);
    len += sprintf(buff + len, "\n");

    if (bgptr) {
        bg_write(bgptr, buff, len);
    }

    return len;
}

static int log_write(void* pri, void* buff, int len)
{
#if defined(_WIN32)
    len = write(_fileno(stdout), buff, len);
#elif defined(__ANDROID__) && defined(DAEMON_JNI_MODE)
    char* pbuf = (char*)buff;
    pbuf[len]  = 0;
    __android_log_print(ANDROID_LOG_INFO, "ar8030", "%s", pbuf);
#else
    len = write(STDOUT_FILENO, buff, len);
#endif
    if (log_fp) {
        fwrite(buff, sizeof(char), len, log_fp);
        fflush(log_fp);
    }
    return len;
}

void com_log_init(char* file)
{
    log_fp = NULL;
    if (file && file[0]) {
        log_fp = fopen(file, "w");
        if (!log_fp) {
            printf("open log file %s , errno = %d\n", file, errno);
        }
    }
    bgptr = bg_init(log_write, log_fp, 100 * 1024, 1024);
}

static int find_idx(com_log_module module)
{
    int sz = sizeof(_infos) / sizeof(_infos[0]);

    for (int i = 0; i < sz; i++) {
        module_info* pinfo = &_infos[i];
        if (pinfo->mod == module) {
            return i;
        }
    }
    return -1;
}

int com_vprintf(const char* fun, com_log_module module, const char* fmt, ...)
{
    int idx = find_idx(module);

    if (idx < 0) {
        return -1;
    }

    int          pri_level = _infos[0].level;
    module_info* pinfo     = _infos + idx;

    if (pinfo->level < pri_level) {
        return -1;
    }

    const char* mod       = pinfo->name_prefix;
    char*       level_str = "unknown";

    switch (pinfo->level) {
    case COM_LOG_DEBUG:
        level_str = "DBG";
        break;
    case COM_LOG_INFO:
        level_str = "INF";
        break;
    case COM_LOG_WARN:
        level_str = "WRN";
        break;
    case COM_LOG_ERROR:
        level_str = "ERR";
        break;
    default:
        break;
    }
    va_list va;
    va_start(va, fmt);
    int len = _com_vprintf(fun, mod, level_str, fmt, va);
    va_end(va);

    return len;
}

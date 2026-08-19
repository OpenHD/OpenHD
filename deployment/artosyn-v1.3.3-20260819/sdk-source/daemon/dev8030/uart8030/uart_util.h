#ifndef __UART_UTIL__
#define __UART_UTIL__

#ifdef __cplusplus
extern "C"
{
#endif

#include "com_log.h"

#define IN
#define OUT

// #define UART_DEBUG

#define uart_log_error(fmt, ...)  com_log(COM_UART_COM, "[%s %d]" fmt"", __FUNCTION__, __LINE__,##__VA_ARGS__);
#define uart_log_info(fmt, ...)   com_log(COM_UART_COM, fmt"", ##__VA_ARGS__);
#define uart_log_buf(fmt, ...)    com_log(COM_UART_COM, "[%s %d]" fmt"", __FUNCTION__, __LINE__,##__VA_ARGS__);
#ifdef UART_DEBUG
#define uart_log_dbg(fmt, ...)    com_log(COM_UART_COM, "[%s %d]" fmt"", __FUNCTION__, __LINE__,##__VA_ARGS__);
#define uart_log_verb(fmt, ...)   com_log(COM_UART_COM, "[%s %d]" fmt"", __FUNCTION__, __LINE__,##__VA_ARGS__);
#define uart_log_warn(fmt, ...)   com_log(COM_UART_COM, "[%s %d]" fmt"", __FUNCTION__, __LINE__,##__VA_ARGS__);
// #define uart_log_buf(fmt, ...)    com_log(COM_UART_COM, "[%s %d]" fmt"", __FUNCTION__, __LINE__,##__VA_ARGS__);
#define HERE                         com_log(COM_UART_COM, "####%s %d ", __FUNCTION__,__LINE__);
#define UART_BUF_PRINT
#else
#define uart_log_dbg(fmt, ...)  do{}while(0);
#define uart_log_warn(fmt, ...) do{}while(0);
#define uart_log_verb(fmt, ...) do{}while(0);
// #define uart_log_buf(fmt, ...)  do{}while(0);
#define HERE
#endif

/* Get the offset of the structure member */
#define VAR_OFFSET(type, mem) ((uint32_t)(&((type *)0)->mem))
/* BIT for uint64 */
#define BIT64(x)    (1ULL << (x))
#define BIT32(x)    (1UL  << (x))

#define min(a, b) (((a) < (b)) ? (a) : (b))

#ifdef __cplusplus
}
#endif

#endif
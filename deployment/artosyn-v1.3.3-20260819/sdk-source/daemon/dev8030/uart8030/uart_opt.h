#pragma once

#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct uart_opt uart_opt;

typedef void* uart_hd;

#include "bb_api.h"

typedef uart_hd (*uart_op)(const char* devname, uart_par* par);
typedef int     (*uart_rd)(uart_hd, void* buff, size_t len);
typedef int     (*uart_wr)(uart_hd, void* buff, size_t len);
typedef int     (*uart_cl)(uart_hd);

typedef uart_list_hd* (*uart_list_alloc)(void);
typedef int           (*uart_list_free)(uart_list_hd*);

typedef struct uart_opt {
    uart_op op;
    uart_rd rd;
    uart_wr wr;
    uart_cl cl;

    uart_list_alloc list_alloc;
    uart_list_free  list_free;
} uart_opt;

uart_opt* get_back_end(void);
#ifdef __cplusplus
}
#endif

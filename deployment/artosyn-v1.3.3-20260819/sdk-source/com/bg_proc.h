#pragma once
#include <stddef.h>

typedef int (*write_proc)(void* pri, void* buff, int len);

typedef struct bg_proc bg_proc;

bg_proc* bg_init(write_proc proc, void* priv, size_t buffsz, size_t batch_sz);
void     bg_write(bg_proc* bgdata, void* data, size_t len);
void     bg_close(bg_proc* bgdata);
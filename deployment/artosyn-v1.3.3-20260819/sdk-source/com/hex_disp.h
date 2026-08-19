#ifndef __HEX_DISP_H__
#define __HEX_DISP_H__

#include <stdint.h>

char* hex_to_string_malloc(const void* buff, int len);
void  hex_to_string_free(char* buff);

#endif

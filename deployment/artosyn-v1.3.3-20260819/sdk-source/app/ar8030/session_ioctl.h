#ifndef __SESSION_IOCTL_H__
#define __SESSION_IOCTL_H__

#include <stdint.h>

struct bb_dev_handle_t;
int ioctl_bb_ioctl(struct bb_dev_handle_t* pdev, uint32_t request, const void* input, void* output, int timeout);

#endif

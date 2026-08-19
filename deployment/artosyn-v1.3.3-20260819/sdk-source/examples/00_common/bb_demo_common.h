#ifndef L4_EXAMPLES_BB_DEMO_COMMON_H_
#define L4_EXAMPLES_BB_DEMO_COMMON_H_

#include "bb_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bb_host_t *host;
    bb_dev_list_t *devs;
    bb_dev_handle_t *handle;
    int dev_count;
    int dev_index;
} bb_demo_context_t;

void bb_demo_context_init(bb_demo_context_t *ctx);
int bb_demo_open(bb_demo_context_t *ctx, const char *addr, int port, int dev_index);
void bb_demo_close(bb_demo_context_t *ctx);
void bb_demo_print_device_info(bb_dev_t *dev, int index);

#ifdef __cplusplus
}
#endif

#endif

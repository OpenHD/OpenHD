#ifndef __SESSION_CALLBACK_H__
#define __SESSION_CALLBACK_H__
#include "session.h"
#include <stdint.h>

struct bb_dev_handle_t;
struct list_head;
int cb_bb_ioctl(struct bb_dev_handle_t* pdev, uint32_t request, const void* input, int timeout);
struct CB_SESSION;
struct CB_SESSION* get_cb_from_node(struct list_head* phead);
struct list_head*  get_node_from_cb(struct CB_SESSION* pcb);
struct BASE_SESSION;
struct BASE_SESSION* get_bs_from_node(struct list_head* phead);
#endif

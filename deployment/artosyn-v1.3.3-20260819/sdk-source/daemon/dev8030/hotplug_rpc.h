#ifndef __HOTPLUG_RPC_H__
#define __HOTPLUG_RPC_H__

#include "bb_api.h"
#include "rpc_node.h"


struct rpc_node* hotplug_cb_alloc(struct threadinfo* tinfo);
void             dev8030_hotplug_event(rpc_info* prpc, bb_event_hotplug_t* pdat);

#endif

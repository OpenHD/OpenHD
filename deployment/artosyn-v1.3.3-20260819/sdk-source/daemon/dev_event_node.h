#ifndef __DEV_EVENT_NODE_H__
#define __DEV_EVENT_NODE_H__

#include "usb_event_list.h"

void dev_event_node_base_add(dev_node_list* plist);
void dev_event_node_del_all(dev_node_list* plist);
void dev_event_node_keep_alive_onec(dev_node_list* plist);
#endif

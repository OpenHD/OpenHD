#ifndef __SESSION_HOTPLUG_H__
#define __SESSION_HOTPLUG_H__
#include "session.h"
#include <stdint.h>

typedef struct bb_host_t bb_host_t;

void bb_dev_del_all_hotplug_cb(bb_host_t* phost);
#endif

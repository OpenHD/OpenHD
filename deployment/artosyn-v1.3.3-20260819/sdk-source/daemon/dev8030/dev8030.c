
#include "dev8030.h"
#include "rpc_node.h"

int dev8030_poll(rpc_info* prpc)
{
    int evt = 0;

    dev8030* polls;
    list_for_each_entry (polls, &prpc->head, node, dev8030) {
        int tmp = polls->pact->polldev(polls);

        if (tmp > 0) {
            evt += tmp;
        }
    }
    return evt;
}

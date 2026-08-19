#include "base_node.h"
#include "usb_event_list.h"
#include <stdio.h>

static int bn_dev_pack_proc(struct basenode* pnod, struct usbpack* pack)
{
    return 0;
}

static PROC_RET bn_dev_pack_chk(struct basenode* pnod, struct usbpack* pack)
{
    return proc_nxt;
}

static int bn_need_write(struct basenode* pnod)
{
    return 0;
}

static int bn_dev_pack_make(struct basenode* pnod, unsigned char* buff, int len)
{
    return -1;
}

static int bn_dev_deinit(basenode* pbn)
{
    if (pbn->clscb) {
        pbn->clscb(pbn, pbn->clbpriv);
    }

    bn_force_close_inotify(pbn);
    return 0;
}

static int bn_dev_cls(basenode* pbn)
{
    bn_set_free(pbn);
    return 0;
}

baseact bnact = {
    .dev_pack_chk   = bn_dev_pack_chk,
    .dev_pack_proc  = bn_dev_pack_proc,
    .dev_write_able = bn_need_write,
    .dev_pack_make  = bn_dev_pack_make,
    .dev_deinit     = bn_dev_deinit,
    .dev_cls        = bn_dev_cls,
};

void bn_init(basenode* pbn, struct dev_node_list* plist, NODE_TYPE tp, baseact const* act)
{
    pbn->freeflag = 0;
    pbn->plist    = plist;

    INIT_LIST_HEAD(&pbn->node);
    INIT_LIST_HEAD(&pbn->cls_node);
    pbn->tp   = tp;
    pbn->pact = act;

    pbn->exitpar = NULL;

    pbn->clscb   = NULL;
    pbn->clbpriv = NULL;
}

void bn_set_free(basenode* pbn)
{
    pbn->freeflag = 1;
    dev_node_tx_inotify(pbn->plist);
}

void bn_set_close_cb(struct basenode* pbn, cls_cb clscb, void* clbpriv)
{
    pbn->clscb   = clscb;
    pbn->clbpriv = clbpriv;
}

void bn_tx_inotify(struct basenode* pbn)
{
    dev_node_list* plist = pbn->plist;

    dev_node_tx_inotify(plist);
}

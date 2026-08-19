#include "base_node.h"
#include "callback_node.h"
#include "com_log.h"
#include "rpc_cmdline.h"
#include "usb_event_list.h"
#include "usbpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#define sleep(n)  Sleep(n * 1000)
#define usleep(n) Sleep(n / 1000)
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

static baseact de_act;

static uint32_t nxt_connect_id = 0;

struct usb_dev_node {
    basenode     base;
    dev_init_seq init_seq;

    char send_error_buff[1024];
    int  send_error_len;
    int  send_error_code;

    char recv_error_buff[1024];
    int  recv_error_len;
    int  recv_error_code;

    int send_keep_alive_once;

    uint32_t connect_id;

    uint32_t print_once : 1;
    uint32_t printed    : 1;
};

static void _dev_pack_proc(struct basenode* pnod, struct usbpack* pack)
{
    struct usb_dev_node* pusb = container_of(pnod, struct usb_dev_node, base);

    switch (pusb->init_seq) {
    case dev_not_init:
    case dev_keep_alive:
        pusb->init_seq = dev_get_remote_bufflen;
        com_log(COM_BASE_COM, "remote alive");
        break;
    case dev_get_remote_bufflen:
        if (pack->subcmdid == 0) {
            unsigned int len = 0;
            len |= (unsigned char)pack->datapack[0] << 0;
            len |= (unsigned char)pack->datapack[1] << 8;

            pnod->plist->remote_buff_len         = len;
            pnod->plist->remote_buff_max_payload = len - get_fix_usblen();

            pnod->plist->remote_max_event_id = (unsigned char)pack->datapack[2] | (unsigned char)pack->datapack[3] << 8;

            memcpy(&pnod->plist->mac, pack->datapack + 4, sizeof(bb_mac_t));

            // 添加callback
            if (pnod->plist->remote_max_event_id > 0) {
                add_base_callback(pnod->plist, pnod->plist->remote_max_event_id);
            }

            rpc_debug_reg(pnod->plist, pnod->plist->mac);

            pusb->init_seq = dev_normal_work;
            com_log(COM_BASE_COM,
                    "remote 1 buffer len = %d , payload max = %d, max event id = %d",
                    pnod->plist->remote_buff_len,
                    pnod->plist->remote_buff_max_payload,
                    pnod->plist->remote_max_event_id);
            if (pnod->plist->devalive) {
                pnod->plist->devalive(pnod->plist, pnod->plist->devalive_priv);
            }
        }
        break;
    case dev_normal_work:
        // printf debug
        if (pack->subcmdid == 0) {
            unsigned int len = 0;
            len |= (unsigned char)pack->datapack[0] << 0;
            len |= (unsigned char)pack->datapack[1] << 8;

            pnod->plist->remote_buff_len         = len;
            pnod->plist->remote_buff_max_payload = len - get_fix_usblen();
            pusb->init_seq                       = dev_normal_work;

            int need_print = 1;

            if (pusb->print_once && pusb->printed) {
                need_print = 0;
            }
            pusb->printed = 1;

            if (need_print) {
                com_log(COM_BASE_COM,
                        "remote 2 buffer len = %d , payload max = %d",
                        pnod->plist->remote_buff_len,
                        pnod->plist->remote_buff_max_payload);
            }
        } else if (pack->subcmdid == 1) {
            com_log(COM_BASE_COM, "remote alive");
        } else if (pack->subcmdid == 2) {

        } else {
            com_log(COM_BASE_COM, "unknown hardware subid = 0x%x", pack->subcmdid);
        }
        break;
    default:
        break;
    }
}

static PROC_RET de_pack_chk(struct basenode* pnod, struct usbpack* pack)
{
    if (pack->domainid == 0xff) {
        return proc_already;
    } else if (pack->domainid == 0xfe) {
        return proc_already;
    } else {
        return proc_nxt;
    }
}

static int de_pack_proc(struct basenode* pnod, struct usbpack* pack)
{
    if (pack->domainid == 0xff) {
        _dev_pack_proc(pnod, pack);
    } else {
        return 0;
    }
    return 1;
}

static int de_need_write(struct basenode* pnod)
{
    struct usb_dev_node* pusb = container_of(pnod, struct usb_dev_node, base);

    if (pusb->send_error_code || pusb->init_seq != dev_normal_work) {
        return 1;
    }

    if (pusb->send_keep_alive_once) {
        return 1;
    }

    return 0;
}

static int de_pack_make(struct basenode* pnod, unsigned char* buff, int len)
{
    struct usb_dev_node* pusb = container_of(pnod, struct usb_dev_node, base);

    usbpack pack = { 0 };
    switch (pusb->init_seq) {
    case dev_not_init:
    case dev_keep_alive:
        pusb->init_seq = dev_keep_alive;
        pack.domainid  = 0xff;
        pack.subcmdid  = 1;
        pack.datalen   = 4;
        pack.data_v    = &pusb->connect_id;
        usleep(10000);
        break;
    case dev_get_remote_bufflen:
        pack.domainid = 0xff;
        pack.subcmdid = 0;
        pack.datalen  = 0;
        pack.datapack = NULL;
        usleep(10000);
        break;
    case dev_normal_work:
        if (pusb->send_keep_alive_once) {
            pusb->send_keep_alive_once = 0;

            pack.domainid = 0xff;
            pack.subcmdid = 2;
            pack.datalen  = 0;
            pack.datapack = NULL;
            break;
        } else if (!pusb->send_error_code) {
            return -1;
        }
        // 这里发送错误代码
        return -2;
    default:
        break;
    }

    return make_usbpack2buff(buff, len, &pack);
}

static int de_dev_deinit(struct basenode* pbase)
{
    com_log(COM_BASE_COM, "free base com act");
    bnact.dev_deinit(pbase);
    free(pbase);
    return 0;
}

static int de_dev_cls(struct basenode* pbase)
{
    bn_set_free(pbase);
    return 0;
}

void dev_event_node_keep_alive_onec(dev_node_list* plist)
{
    // free hardware
    struct list_head* phead = &plist->hard_list;

    basenode* nodetst;
    list_for_each_entry (nodetst, &plist->hard_list, node, basenode) {
        struct usb_dev_node* pusb  = container_of(nodetst, struct usb_dev_node, base);
        pusb->send_keep_alive_once = 1;
    }
}

void dev_event_node_base_add(dev_node_list* plist)
{
    struct usb_dev_node* pusb = malloc(sizeof(struct usb_dev_node));
    if (!pusb) {
        com_log(COM_BASE_COM, "malloc usb_deb_node err");
        return;
    }
    bn_init(&pusb->base, plist, nod_hardware, &de_act);

    pusb->send_error_code = 0;
    pusb->init_seq        = dev_not_init;

    pusb->send_keep_alive_once = 0;

    pusb->print_once = 1;
    pusb->printed    = 0;

    if (nxt_connect_id == 0) {
#ifdef BUILD_DAEMON_KO
        get_random_bytes(&nxt_connect_id, 4);
#else
        srand(time(NULL));
        nxt_connect_id = rand();
#endif
    }

    pusb->connect_id = nxt_connect_id++;

    com_log(COM_BASE_COM, "add new connect_id %u", pusb->connect_id);

    plist_insert_node(plist, &pusb->base);
}

void dev_event_node_del_all(dev_node_list* plist)
{
    // free hardware
    struct list_head* phead = &plist->hard_list;

    while (!list_empty(phead)) {
        basenode* nodetst = list_first_entry(phead, basenode, node);
        list_del(&nodetst->node);
        if (nodetst->pact->dev_deinit) {
            nodetst->pact->dev_deinit(nodetst);
        }
    }
}

static baseact de_act = {
    .dev_pack_chk   = de_pack_chk,
    .dev_pack_proc  = de_pack_proc,
    .dev_write_able = de_need_write,
    .dev_pack_make  = de_pack_make,
    .dev_deinit     = de_dev_deinit,
    .dev_cls        = de_dev_cls,
};

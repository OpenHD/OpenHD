#include "usb_event_list.h"
#include "base_node.h"
#include "com_log.h"
#include "dev_event_node.h"
#include "timspec_helper.h"
#include "usbpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int dev_node_get_tmp_msgid(dev_node_list* plist)
{
    pthread_spin_lock(&plist->msgid_lk);
    int ret = ++plist->msgid_tmp;
    pthread_spin_unlock(&plist->msgid_lk);
    return ret;
}

static int find_chk(basenode* pnod, void* par)
{
    usbpack* pack = (usbpack*)par;
    if (!pnod || !pnod->pact->dev_pack_chk) {
        return proc_nxt;
    }

    return pnod->pact->dev_pack_chk(pnod, pack) == proc_already;
}

typedef struct exit_str {
    pthread_cond_t   close_cv;
    struct list_head cls_head;
} exit_str;

void bn_force_close_inotify(basenode* pbn)
{
    pthread_mutex_lock(&pbn->plist->mtx);
    if (pbn->exitpar) {
        exit_str* pext = pbn->exitpar;
        list_del(&pbn->cls_node);
        pthread_cond_broadcast(&pext->close_cv);
    }
    pthread_mutex_unlock(&pbn->plist->mtx);
}

static void del_worknod(dev_node_list* plist)
{
    exit_str ext;
    INIT_LIST_HEAD(&ext.cls_head);
    pthread_cond_init(&ext.close_cv, NULL);

    pthread_mutex_lock(&plist->mtx);
    struct list_head* phead = &plist->work_list;

    basenode* nodetst;
    list_for_each_entry (nodetst, phead, node, basenode) {
        nodetst->exitpar = &ext;
        list_add(&nodetst->cls_node, &ext.cls_head);
    }

    list_for_each_entry (nodetst, phead, node, basenode) {
        if (nodetst->pact->dev_nm && nodetst->pact->dev_nm(nodetst)) {
            com_log(COM_NET, "force del work node %s", nodetst->pact->dev_nm(nodetst));
        }

        if (nodetst->pact->dev_cls) {
            nodetst->pact->dev_cls(nodetst);
        }
    }
    // 确保清空
    while (!list_empty(&ext.cls_head)) {
        pthread_cond_wait(&ext.close_cv, &plist->mtx);
    }

    pthread_mutex_unlock(&plist->mtx);

    pthread_cond_destroy(&ext.close_cv);
}

static int find_work_and_del(basenode* pnod, void* par)
{
    if (!pnod) {
        return 0;
    }

    if (pnod->freeflag) {
        return 1;
    }

    if (!pnod->pact->dev_write_able) {
        return 0;
    }

    return pnod->pact->dev_write_able(pnod);
}

struct basenode* plist_find_work_node(dev_node_list* plist, finder fd, void* fdpar)
{
    if (!plist || !fd) {
        return NULL;
    }

    struct list_head* phead = &plist->work_list;
    basenode*         ptest;
    list_for_each_entry (ptest, phead, node, basenode) {
        if (fd(ptest, fdpar)) {
            return ptest;
        }
    }
    return NULL;
}

static struct basenode* plist_find_usb_node(dev_node_list* plist, finder fd, void* fdpar)
{
    struct list_head* phead = &plist->hard_list;
    basenode*         ptest;
    list_for_each_entry (ptest, phead, node, basenode) {
        if (fd(ptest, fdpar)) {
            return ptest;
        }
    }
    return NULL;
}

static basenode* get_working_node(dev_node_list* plist)
{
    while (plist->running & run_dev) {
        basenode* ret = plist_find_usb_node(plist, find_work_and_del, NULL);
        if (!ret) {
            ret = plist_find_work_node(plist, find_work_and_del, NULL);
        }

        if (ret) {
            return ret;
        } else if (plist->scanflg) {
            plist->scanflg = 0;
            continue;
        } else {
            struct timespec tp;
            get_timespec_from_now(&tp, 100);
            int sta = pthread_cond_timedwait(&plist->cv, &plist->mtx, &tp);

            if (sta == ETIMEDOUT) {
                // 触发keep alive数据
                dev_event_node_keep_alive_onec(plist);
            }
        }
    }
    return NULL;
}

static void* dev_send_thread(void* p)
{
#define SD_BUF_LEN (100 * 1024)
    unsigned char* sndbuff = (unsigned char*)malloc(SD_BUF_LEN);

    dev_node_list* plist = (dev_node_list*)p;
    char           names[16];
    strcpy(names, "unknown");
    if (plist->devname) {
        plist->devname(plist, names, 16);
    }

    com_log(COM_NET, "%s dev send start", names);
    while (plist->running & run_dev) {
        pthread_mutex_lock(&plist->mtx);
        basenode* tmpnod = get_working_node(plist);
        if (tmpnod && tmpnod->freeflag) {
            plist_del_nod(plist, tmpnod);
        }
        pthread_mutex_unlock(&plist->mtx);

        if (tmpnod && tmpnod->freeflag) {
            if (tmpnod->pact->dev_deinit) {
                tmpnod->pact->dev_deinit(tmpnod);
            }
            continue;
        }

        if (!tmpnod) {
            continue;
        }

        while (plist->running & run_dev) {
            if (tmpnod->freeflag) {
                break;
            }

            if (!tmpnod->pact->dev_pack_make) {
                break;
            }

            int sdlen = tmpnod->pact->dev_pack_make(tmpnod, sndbuff, SD_BUF_LEN);

            if (sdlen > 0) {
                plist->dev_send(plist->devhandle, sndbuff, sdlen, 0);
            } else {
                break;
            }
        }
    }
    com_log(COM_NET, "%s dev send end", names);
    free(sndbuff);

    return NULL;
}

static void check_recv_pack(dev_node_list* plist, usbpack* pack)
{
    basenode* head = plist_find_usb_node(plist, find_chk, pack);

    if (!head) {
        pthread_mutex_lock(&plist->mtx);
        head = plist_find_work_node(plist, find_chk, pack);
        pthread_mutex_unlock(&plist->mtx);
    }

    if (head && head->pact->dev_pack_proc) {
        head->pact->dev_pack_proc(head, pack);
    } else {
        com_log(COM_NET, "dev pack get null proc , reqid = %x", pack->reqid);
    }
}

static int plist_usb_rd_cb(void* priv, unsigned char* rd_buff, int bufflen)
{
    dev_node_list* plist = (dev_node_list*)priv;

    int usedlen = 0;
    int ret     = 0;
    while (1) {
        int     curusedlen;
        usbpack pack;
        ret = unpack_usb_pack(rd_buff + usedlen, bufflen - usedlen, &curusedlen, &pack);

        if (ret < 0) {
            break;
        }

        check_recv_pack(plist, &pack);
        usedlen += curusedlen;
    }
    return ret;
}

static void* dev_recv_thread(void* p)
{
    dev_node_list* plist = (dev_node_list*)p;
    plist->recv_priv     = plist;
    plist->dev_rd_cb     = plist_usb_rd_cb;
    char names[16];
    strcpy(names, "unknown");
    if (plist->devname) {
        plist->devname(plist, names, 16);
    }
    com_log(COM_NET, "%s dev recv start", names);
    while (plist->running & run_dev) {
        plist->dev_loop(plist->devhandle);
    }
    com_log(COM_NET, "%s dev recv end", names);
    return NULL;
}

void plist_insert_node(dev_node_list* plist, basenode* nod)
{
    if (!plist || !nod) {
        return;
    }
    NODE_TYPE tp = nod->tp;
    if (tp >= nod_max || tp < 0) {
        return;
    }

    if (tp == nod_not_init) {
        return;
    }
    pthread_mutex_lock(&plist->mtx);
    struct list_head* phead = nod->tp == nod_hardware ? &plist->hard_list : &plist->work_list;

    list_add(&nod->node, phead);
    pthread_mutex_unlock(&plist->mtx);
}

void plist_del_nod(dev_node_list* plist, basenode* nod)
{
    if (!plist || !nod) {
        return;
    }

    list_del(&nod->node);
}

void dev_node_list_init(dev_node_list* plist)
{
    // init list
    INIT_LIST_HEAD(&plist->work_list);
    INIT_LIST_HEAD(&plist->hard_list);

    pthread_mutexattr_init(&plist->mtx_attr);
    pthread_mutexattr_settype(&plist->mtx_attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&plist->mtx, &plist->mtx_attr);
    pthread_cond_init(&plist->cv, NULL);

    dev_event_node_base_add(plist);

    pthread_create(&plist->t_send, NULL, dev_send_thread, plist);
    pthread_create(&plist->t_recv, NULL, dev_recv_thread, plist);

    plist->msgid_tmp = 0;
    pthread_spin_init(&plist->msgid_lk, PTHREAD_PROCESS_PRIVATE);
}

static void dev_node_list_lock_deinit(dev_node_list* plist)
{
    pthread_mutex_destroy(&plist->mtx);
    pthread_cond_destroy(&plist->cv);

    pthread_mutexattr_destroy(&plist->mtx_attr);

    pthread_spin_destroy(&plist->msgid_lk);
}

static void dev_node_list_wait_end(dev_node_list* plist)
{
    pthread_join(plist->t_send, NULL);
    pthread_join(plist->t_recv, NULL);
}

int dev_node_list_check_running(dev_node_list* plist)
{
    return plist->running;
}

int dev_node_list_get_buff(dev_node_list* plist)
{
    return plist->remote_buff_len;
}

void dev_node_tx_inotify(dev_node_list* plist)
{
    pthread_mutex_lock(&plist->mtx);
    plist->scanflg = 1;
    pthread_cond_broadcast(&plist->cv);
    pthread_mutex_unlock(&plist->mtx);
}

/**
 * @brief 结束所有 usb事务
 *
 * @param plist
 */
void dev_node_force_exit(dev_node_list* plist)
{
    com_log(COM_HOT_PLUG_COM, "dev_node_force_exit del all node");

    // 清理 dev node
    del_worknod(plist);

    com_log(COM_HOT_PLUG_COM, "dev_node_force_exit stop subdev");
    // 清理 usb tx \ rx
    plist->running &= ~run_dev;
    dev_node_tx_inotify(plist);
    dev_node_list_wait_end(plist);

    dev_event_node_del_all(plist);

    dev_node_list_lock_deinit(plist);

    com_log(COM_HOT_PLUG_COM, "stop dev_node_list@%p", plist);
    free(plist);
}

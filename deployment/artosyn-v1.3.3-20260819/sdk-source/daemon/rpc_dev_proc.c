#include "dev8030.h"
#include "rpc_node.h"
#include <string.h>

void rpc_dev_add(rpc_info* pinfo, dev8030* pdev)
{
    pthread_rwlock_wrlock(&pinfo->head_lk);
    list_add_tail(&pdev->node, &pinfo->head);
    pthread_rwlock_unlock(&pinfo->head_lk);
}

int rpc_dev_getsz(rpc_info* prpc_info)
{
    int ret = 0;
    pthread_rwlock_rdlock(&prpc_info->head_lk);
    dev8030* testdev;
    list_for_each_entry (testdev, &prpc_info->head, node, dev8030) {
        int tmp = testdev->pact->getsz(testdev);
        if (tmp >= 0) {
            ret += tmp;
        }
    }
    pthread_rwlock_unlock(&prpc_info->head_lk);
    return ret;
}

int rpc_dev_getid_all(rpc_info* prpc_info, uint32_t* ids, size_t sz)
{
    int cursz = 0;
    pthread_rwlock_rdlock(&prpc_info->head_lk);
    dev8030* testdev;

    list_for_each_entry (testdev, &prpc_info->head, node, dev8030) {
        cursz += testdev->pact->getid_all(testdev, ids + cursz, sz - cursz);
    }
    pthread_rwlock_unlock(&prpc_info->head_lk);

    return cursz;
}

int rpc_dev_fill_serial(rpc_info* prpc_info, uint32_t workid, uint8_t* buff, size_t len)
{
    int ret = -1;
    pthread_rwlock_rdlock(&prpc_info->head_lk);
    dev8030* testdev;
    list_for_each_entry (testdev, &prpc_info->head, node, dev8030) {
        ret = testdev->pact->fill_serial(testdev, workid, buff, len);
        if (ret >= 0) {
            break;
        }
    }
    pthread_rwlock_unlock(&prpc_info->head_lk);

    return ret;
}

void* rpc_dev_get_plist(rpc_info* prpc_info, uint32_t workid)
{
    void* ret = NULL;
    pthread_rwlock_rdlock(&prpc_info->head_lk);
    dev8030* testdev;
    list_for_each_entry (testdev, &prpc_info->head, node, dev8030) {
        ret = testdev->pact->getplist(testdev, workid);
        if (ret) {
            break;
        }
    }
    pthread_rwlock_unlock(&prpc_info->head_lk);

    return ret;
}

int rpc_dev_get_nxt_working_id(rpc_info* prpc_info)
{
    return prpc_info->nxt_working_id++;
}

dev8030* rpc_get_plat_by_name(rpc_info* prpc_info, const char* name)
{
    int succflg = 0;
    pthread_rwlock_rdlock(&prpc_info->head_lk);
    dev8030* testdev;
    list_for_each_entry (testdev, &prpc_info->head, node, dev8030) {
        if (!strcmp(testdev->pact->name, name)) {
            succflg = 1;
            break;
        }
    }
    pthread_rwlock_unlock(&prpc_info->head_lk);
    return succflg ? testdev : NULL;
}

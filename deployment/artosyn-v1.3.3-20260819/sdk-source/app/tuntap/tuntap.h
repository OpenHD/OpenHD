#ifndef __L4_TUNTAP_H__
#define __L4_TUNTAP_H__

#include "bb_api.h"
#include "tuntap++.hh"
#include "string.h"
#include <atomic>
#include <memory>
#include <mutex>
struct bb_tun_cfg {
    // bb field
    std::atomic<int> bb_fd {-1};
    std::mutex       bb_socket_mutex;

    bb_host_t*       phost   = nullptr;
    bb_dev_handle_t* pdev    = nullptr;
    bb_dev_info_t    target_dev_info = {};
    bool             target_dev_info_valid = false;
    bb_slot_e        slot_id = BB_SLOT_0;
    int              port_id = 3;
    int              dev_index = 0;
    int              daemon_port = BB_PORT_DEFAULT;
    char             addr[128] = { 0 };
    // tun field
    int  ipset_flg    = 0;
    int  mtu          = 4000;
    char devname[128] = { 0 };
    char ip[128]      = { 0 };
    char mask[128]    = { 0 };
    // int  tun_fd       = -1;
    std::unique_ptr<tuntap::tap> dev;
    // common
    int debugflg = 0;
    int force_close_on_open_fail = 0;
    int buff_max = 4096;
    uint32_t rx_buf_len = 40000;
    uint32_t tx_buf_len = 60000;

    bb_tun_cfg()
    {
        strcpy(addr, "127.0.0.1");
        strcpy(devname, "tap0");
        ip[0] = '\0';
        strcpy(mask, "255.255.255.0");
    }
};

#endif

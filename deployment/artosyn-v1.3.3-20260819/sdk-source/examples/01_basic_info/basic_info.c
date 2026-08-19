#include "bb_demo_common.h"

#include "getopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *role_name(uint8_t role)
{
    switch (role) {
    case BB_ROLE_AP:
        return "AP";
    case BB_ROLE_DEV:
        return "DEV";
    default:
        return "UNKNOWN";
    }
}

static const char *mode_name(uint8_t mode)
{
    switch (mode) {
    case BB_MODE_SINGLE_USER:
        return "SINGLE_USER";
    case BB_MODE_MULTI_USER:
        return "MULTI_USER";
    case BB_MODE_RELAY:
        return "RELAY";
    case BB_MODE_DIRECTOR:
        return "DIRECTOR";
    default:
        return "UNKNOWN";
    }
}

static const char *link_state_name(uint8_t state)
{
    switch (state) {
    case BB_LINK_STATE_IDLE:
        return "IDLE";
    case BB_LINK_STATE_LOCK:
        return "LOCK";
    case BB_LINK_STATE_CONNECT:
        return "CONNECT";
    default:
        return "UNKNOWN";
    }
}

static const char *bandwidth_name(uint8_t bandwidth)
{
    switch (bandwidth) {
    case BB_BW_1_25M:
        return "1.25MHz";
    case BB_BW_2_5M:
        return "2.5MHz";
    case BB_BW_5M:
        return "5MHz";
    case BB_BW_10M:
        return "10MHz";
    case BB_BW_20M:
        return "20MHz";
    case BB_BW_40M:
        return "40MHz";
    default:
        return "UNKNOWN";
    }
}

static const char *tintlv_mode_name(uint8_t tintlv_len, uint8_t tintlv_num)
{
    if (tintlv_len == 3 && tintlv_num == 1) {
        return "Y24X2";
    }

    if (tintlv_len == 2 && tintlv_num == 0) {
        return "Y12X1";
    }

    return "UNKNOWN";
}

static const char *tintlv_major_dir(uint8_t tintlv_len, uint8_t tintlv_num)
{
    if (tintlv_len == 3 && tintlv_num == 1) {
        return "DEV->AP";
    }

    if (tintlv_len == 2 && tintlv_num == 0) {
        return "AP->DEV";
    }

    return "UNKNOWN";
}

typedef struct {
    int user;
    const char *source;
    const bb_phy_status_t *phy;
} user_phy_view_t;

static int get_user_phy_view(const bb_get_status_out_t *status, int tx, user_phy_view_t *view)
{
    switch (status->role) {
    case BB_ROLE_AP:
        view->user = tx ? BB_USER_BR_CS : BB_USER_0;
        view->source = tx ? "tx" : "rx";
        break;
    case BB_ROLE_DEV:
        view->user = tx ? BB_USER_0 : BB_USER_BR_CS;
        view->source = tx ? "tx" : "rx";
        break;
    default:
        return -1;
    }

    view->phy = tx ? &status->user_status[view->user].tx_status :
                     &status->user_status[view->user].rx_status;
    return 0;
}

static void print_user_phy_view(const char *dir, const user_phy_view_t *view, int tx)
{
    const bb_phy_status_t *phy = view->phy;

    if (tx) {
        printf("  %s: user=%d source=%s freq=%uKHz mcs_raw=%u mcs_real=%d bw=%s(%u) "
               "tintlv_enable=%u tintlv_len=%u tintlv_num=%u\n",
               dir,
               view->user,
               view->source,
               phy->freq_khz,
               phy->mcs,
               (int)phy->mcs - 2,
               bandwidth_name(phy->bandwidth),
               phy->bandwidth,
               phy->tintlv_enable,
               phy->tintlv_len,
               phy->tintlv_num);
        return;
    }

    printf("  %s: user=%d source=%s freq=%uKHz bw=%s(%u) tintlv_enable=%u tintlv_len=%u "
           "tintlv_num=%u\n",
           dir,
           view->user,
           view->source,
           phy->freq_khz,
           bandwidth_name(phy->bandwidth),
           phy->bandwidth,
           phy->tintlv_enable,
           phy->tintlv_len,
           phy->tintlv_num);
}

static void print_user_phy_status(const bb_get_status_out_t *status)
{
    user_phy_view_t rx_view;
    user_phy_view_t tx_view;
    const char *mode;
    const char *major_dir;

    printf("\nuser phy status:\n");
    if (get_user_phy_view(status, 0, &rx_view) || get_user_phy_view(status, 1, &tx_view)) {
        printf("  unavailable: unsupported role=%u\n", status->role);
        return;
    }

    mode = tintlv_mode_name(rx_view.phy->tintlv_len, rx_view.phy->tintlv_num);
    major_dir = tintlv_major_dir(rx_view.phy->tintlv_len, rx_view.phy->tintlv_num);

    print_user_phy_view("RX", &rx_view, 0);
    print_user_phy_view("TX", &tx_view, 1);
    printf("  bw_mode=%s major_dir=%s\n", mode, major_dir);
}

static void print_mac(const bb_mac_t *mac)
{
    for (int i = 0; i < BB_MAC_LEN; ++i) {
        printf("%s%02x", i == 0 ? "" : ":", mac->addr[i]);
    }
}

static void usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  -h              show this help\n");
    printf("  -a <addr>       daemon address, default: 127.0.0.1\n");
    printf("  -p <port>       daemon port, default: %d\n", BB_PORT_DEFAULT);
    printf("  -i <index>      device index, default: 0\n");
    printf("  -l              list devices and exit\n");
    printf("  -s <slot>       remote slot id for -R, default: 0; DEV uses slot 0 as AP\n");
    printf("  -S              query BB_GET_STATUS\n");
    printf("  -V              query BB_GET_SYS_INFO\n");
    printf("  -R              query peer device by remote ioctl through -s <slot>\n");
    printf("  -A              query all basic information, default action\n");
}

static int list_devices(const char *addr, int port)
{
    bb_demo_context_t ctx;
    int ret;

    bb_demo_context_init(&ctx);

    printf("connect host %s:%d\n", addr, port);
    ret = bb_host_connect(&ctx.host, addr, port);
    if (ret) {
        printf("bb_host_connect failed, ret=%d\n", ret);
        return ret;
    }

    ctx.dev_count = bb_dev_getlist(ctx.host, &ctx.devs);
    if (ctx.dev_count <= 0) {
        printf("bb_dev_getlist found no device, ret=%d\n", ctx.dev_count);
        bb_demo_close(&ctx);
        return -1;
    }

    printf("device count: %d\n", ctx.dev_count);
    for (int i = 0; i < ctx.dev_count; ++i) {
        bb_demo_print_device_info(ctx.devs[i], i);
    }

    bb_demo_close(&ctx);
    return 0;
}

static int basic_info_ioctl(bb_dev_handle_t *handle,
                            uint32_t request,
                            const void *input,
                            uint16_t input_len,
                            void *output,
                            uint16_t output_len,
                            int remote_slot)
{
    bb_remote_ioctl_in_t remote_in;
    bb_remote_ioctl_out_t remote_out;
    int ret;

    if (remote_slot < 0) {
        return bb_ioctl(handle, request, input, output);
    }

    if ((input_len && !input) || input_len > sizeof(remote_in.data) ||
        output_len > sizeof(remote_out.data)) {
        printf("remote ioctl buffer invalid, request=0x%x in_len=%u out_len=%u\n",
               request,
               input_len,
               output_len);
        return -1;
    }

    memset(&remote_in, 0, sizeof(remote_in));
    memset(&remote_out, 0, sizeof(remote_out));

    if (input_len) {
        memcpy(remote_in.data, input, input_len);
    }

    remote_in.len = input_len;
    remote_in.slot = (uint8_t)remote_slot;
    remote_in.msg_id = request;

    ret = bb_ioctl(handle, BB_REMOTE_IOCTL_REQ, &remote_in, &remote_out);
    if (ret) {
        return ret;
    }

    if (output && output_len) {
        memcpy(output, remote_out.data, output_len);
    }

    return 0;
}

static int query_sys_info(bb_dev_handle_t *handle, int remote_slot)
{
    bb_get_sys_info_out_t info;
    int ret;

    memset(&info, 0, sizeof(info));
    ret = basic_info_ioctl(handle,
                           BB_GET_SYS_INFO,
                           NULL,
                           0,
                           &info,
                           sizeof(info),
                           remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_SYS_INFO remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_SYS_INFO failed, ret=%d\n", ret);
        }
        return ret;
    }

    if (remote_slot >= 0) {
        printf("\n[BB_GET_SYS_INFO remote slot=%d]\n", remote_slot);
    } else {
        printf("\n[BB_GET_SYS_INFO]\n");
    }
    printf("uptime       : %llu ms\n", (unsigned long long)info.uptime);
    printf("compile_time : %s\n", info.compile_time);
    printf("soft_ver     : %s\n", info.soft_ver);
    printf("hardware_ver : %s\n", info.hardware_ver);
    printf("firmware_ver : %s\n", info.firmware_ver);
    return 0;
}

static int query_status(bb_dev_handle_t *handle, int remote_slot)
{
    bb_get_status_in_t input;
    bb_get_status_out_t status;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&status, 0, sizeof(status));

    input.user_bmp = 0xffff;
    ret = basic_info_ioctl(handle,
                           BB_GET_STATUS,
                           &input,
                           sizeof(input),
                           &status,
                           sizeof(status),
                           remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_STATUS remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_STATUS failed, ret=%d\n", ret);
        }
        return ret;
    }

    if (remote_slot >= 0) {
        printf("\n[BB_GET_STATUS remote slot=%d]\n", remote_slot);
    } else {
        printf("\n[BB_GET_STATUS]\n");
    }
    printf("role        : %s (%u)\n", role_name(status.role), status.role);
    printf("mode        : %s (%u)\n", mode_name(status.mode), status.mode);
    printf("sync_mode   : %u\n", status.sync_mode);
    printf("sync_master : %u\n", status.sync_master);
    printf("cfg_sbmp    : 0x%02x\n", status.cfg_sbmp);
    printf("rt_sbmp     : 0x%02x\n", status.rt_sbmp);
    printf("local_mac   : ");
    print_mac(&status.mac);
    printf("\n");

    printf("\nslot link status:\n");
    for (int i = 0; i < (status.mode == BB_MODE_SINGLE_USER ? 1 : BB_SLOT_MAX); ++i) {
        const bb_link_status_t *link = &status.link_status[i];
        if (link->state != BB_LINK_STATE_IDLE &&
            link->state != BB_LINK_STATE_LOCK &&
            link->state != BB_LINK_STATE_CONNECT) {
            continue;
        }

        printf("  slot[%d]: state=%s(%u), rx_mcs_raw=%u, rx_mcs_real=%d, pair=%u, peer_mac=",
               i,
               link_state_name(link->state),
               link->state,
               link->rx_mcs,
               (int)link->rx_mcs - 2,
               link->pair_state);
        print_mac(&link->peer_mac);
        printf("\n");
    }

    print_user_phy_status(&status);

    return 0;
}
int main(int argc, char **argv)
{
    const char *addr = "127.0.0.1";
    int port = BB_PORT_DEFAULT;
    int dev_index = 0;
    int slot = 0;
    int do_status = 0;
    int do_sys_info = 0;
    int remote = 0;
    int list_only = 0;
    int opt;
    int ret;
    bb_demo_context_t ctx;

    while ((opt = getopt(argc, argv, "ha:p:i:ls:SVRA")) != -1) {
        switch (opt) {
        case 'h':
            usage(argv[0]);
            return 0;
        case 'a':
            addr = optarg;
            break;
        case 'p':
            port = (int)strtol(optarg, NULL, 10);
            break;
        case 'i':
            dev_index = (int)strtol(optarg, NULL, 10);
            break;
        case 'l':
            list_only = 1;
            break;
        case 's':
            slot = (int)strtol(optarg, NULL, 10);
            break;
        case 'S':
            do_status = 1;
            break;
        case 'V':
            do_sys_info = 1;
            break;
        case 'R':
            remote = 1;
            break;
        case 'A':
            do_status = 1;
            do_sys_info = 1;
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    if (list_only) {
        ret = list_devices(addr, port);
        return ret ? -1 : 0;
    }

    if (!do_status && !do_sys_info) {
        do_status = 1;
        do_sys_info = 1;
    }

    if (slot < 0 || slot >= BB_SLOT_MAX) {
        printf("invalid slot %d, valid range: 0-%d\n", slot, BB_SLOT_MAX - 1);
        return -1;
    }

    ret = bb_demo_open(&ctx, addr, port, dev_index);
    if (ret) {
        return -1;
    }

    if (do_sys_info) {
        ret = query_sys_info(ctx.handle, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_status) {
        ret = query_status(ctx.handle, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    printf("\nl4 basic info example finished\n");

done:
    bb_demo_close(&ctx);
    return ret ? -1 : 0;
}

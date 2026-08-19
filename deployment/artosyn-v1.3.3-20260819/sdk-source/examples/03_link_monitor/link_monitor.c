#include "bb_demo_common.h"

#include "getopt.h"
#include <math.h>
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

static const char *dir_name(uint8_t dir)
{
    switch (dir) {
    case BB_DIR_TX:
        return "TX";
    case BB_DIR_RX:
        return "RX";
    default:
        return "UNKNOWN";
    }
}

static const char *band_name(uint8_t band)
{
    switch (band) {
    case BB_BAND_1G:
        return "1G";
    case BB_BAND_2G:
        return "2G";
    case BB_BAND_5G:
        return "5G";
    default:
        return "UNKNOWN";
    }
}

static const char *auto_mode_name(uint8_t mode)
{
    switch (mode) {
    case 0:
        return "MANUAL";
    case 1:
        return "AUTO";
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

static double snr_to_db(uint16_t snr)
{
    if (snr == 0) {
        return 0.0;
    }

    return 10.0 * log10((double)snr / 36.0);
}

static void print_quality(const char *prefix, const bb_quality_t *quality)
{
    printf("%s snr_raw=%u snr_db=%.2f dB ldpc=%u/%u gain_a=%u gain_b=%u\n",
           prefix,
           quality->snr,
           snr_to_db(quality->snr),
           quality->ldpc_err,
           quality->ldpc_num,
           quality->gain_a,
           quality->gain_b);
}

static void print_1v1_info(const char *name, const bb_info_t *info)
{
    printf("  %s: snr_raw=%u snr_db=%.2f dB ldpc_tlv_err_ratio=%u "
           "ldpc_num_err_ratio=%u gain_a=%u gain_b=%u tx_mcs=%u tx_chan=%u "
           "tx_power=%u tx_freq_khz=%u\n",
           name,
           info->snr,
           snr_to_db(info->snr),
           info->ldpc_tlv_err_ratio,
           info->ldpc_num_err_ratio,
           info->gain_a,
           info->gain_b,
           info->tx_mcs,
           info->tx_chan,
           info->tx_power,
           info->tx_freq_khz);
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
    printf("  -s <slot>       slot id, default: 0; DEV uses slot 0 as AP\n");
    printf("  -u <user>       physical user id; -Q default: AP=0, DEV=8; -P default: AP=8, DEV=0\n");
    printf("  -A              query all link information, default action\n");
    printf("  -S              query BB_GET_STATUS\n");
    printf("  -Q              query BB_GET_USER_QUALITY\n");
    printf("  -q              query BB_GET_PEER_QUALITY\n");
    printf("  -M              query BB_GET_MCS for TX and RX\n");
    printf("  -m              query BB_GET_MCS_MODE\n");
    printf("  -W              query BB_GET_BANDWIDTH_MODE\n");
    printf("  -P              query BB_GET_CUR_POWER\n");
    printf("  -C              query BB_GET_CHAN_INFO\n");
    printf("  -R              query peer device by remote ioctl through -s <slot>\n");
    printf("  -B              query BB_GET_BAND_INFO\n");
    printf("  -T              query BB_GET_THROUGHPUT for TX and RX\n");
    printf("  -D              query BB_GET_DISTC_RESULT\n");
    printf("  -V              query BB_GET_1V1_INFO\n");
}

static int link_monitor_ioctl(bb_dev_handle_t *handle,
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

static void print_query_title(const char *title, int remote_slot)
{
    if (remote_slot >= 0) {
        printf("\n[%s remote slot=%d]\n", title, remote_slot);
        return;
    }

    printf("\n[%s]\n", title);
}

static int read_status(bb_dev_handle_t *handle, bb_get_status_out_t *status, int remote_slot)
{
    bb_get_status_in_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(status, 0, sizeof(*status));

    input.user_bmp = 0xffff;
    ret = link_monitor_ioctl(handle,
                             BB_GET_STATUS,
                             &input,
                             sizeof(input),
                             status,
                             sizeof(*status),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_STATUS remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_STATUS failed, ret=%d\n", ret);
        }
        return ret;
    }

    return 0;
}

static void print_status(const bb_get_status_out_t *status, int remote_slot)
{
    print_query_title("BB_GET_STATUS", remote_slot);
    printf("role        : %s (%u)\n", role_name(status->role), status->role);
    printf("mode        : %s (%u)\n", mode_name(status->mode), status->mode);
    printf("sync_mode   : %u\n", status->sync_mode);
    printf("sync_master : %u\n", status->sync_master);
    printf("cfg_sbmp    : 0x%02x\n", status->cfg_sbmp);
    printf("rt_sbmp     : 0x%02x\n", status->rt_sbmp);
    printf("local_mac   : ");
    print_mac(&status->mac);
    printf("\n");

    printf("\nslot link status:\n");
    for (int i = 0; i < (status->mode == BB_MODE_SINGLE_USER ? 1 : BB_SLOT_MAX); ++i) {
        const bb_link_status_t *link = &status->link_status[i];
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

    print_user_phy_status(status);
}

static int query_status(bb_dev_handle_t *handle, int remote_slot)
{
    bb_get_status_out_t status;
    int ret;

    ret = read_status(handle, &status, remote_slot);
    if (ret) {
        return ret;
    }

    print_status(&status, remote_slot);
    return 0;
}

static int link_ready_for_slot(const bb_get_status_out_t *status, int slot)
{
    const bb_link_status_t *link;

    if (status->role != BB_ROLE_AP && status->role != BB_ROLE_DEV) {
        return 0;
    }

    link = &status->link_status[slot];
    return link->pair_state || link->state == BB_LINK_STATE_CONNECT;
}

static int default_quality_user_for_role(uint8_t role)
{
    switch (role) {
    case BB_ROLE_AP:
        return BB_USER_0;
    case BB_ROLE_DEV:
        return BB_USER_BR_CS;
    default:
        return BB_USER_0;
    }
}

static int default_power_user_for_role(uint8_t role)
{
    switch (role) {
    case BB_ROLE_AP:
        return BB_USER_BR_CS;
    case BB_ROLE_DEV:
        return BB_USER_0;
    default:
        return BB_USER_0;
    }
}

static int query_user_quality(bb_dev_handle_t *handle, int user, int remote_slot)
{
    bb_get_user_quality_in_t input;
    bb_get_user_quality_out_t output;
    char prefix[32];
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.user_bmp = (uint16_t)(1u << user);
    input.average = 0;
    ret = link_monitor_ioctl(handle,
                             BB_GET_USER_QUALITY,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_USER_QUALITY remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_USER_QUALITY failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_USER_QUALITY", remote_slot);
    snprintf(prefix, sizeof(prefix), "user[%d]:", user);
    print_quality(prefix, &output.qualities[user]);
    return 0;
}

static int query_peer_quality(bb_dev_handle_t *handle, int slot, int remote_slot)
{
    bb_get_peer_quality_in_t input;
    bb_get_peer_quality_out_t output;
    char prefix[32];
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.slot_bmp = (uint8_t)(1u << slot);
    input.arverage = 0;
    ret = link_monitor_ioctl(handle,
                             BB_GET_PEER_QUALITY,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_PEER_QUALITY remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_PEER_QUALITY failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_PEER_QUALITY", remote_slot);
    snprintf(prefix, sizeof(prefix), "slot[%d]:", slot);
    print_quality(prefix, &output.qualities[slot]);
    return 0;
}

static int query_mcs_dir(bb_dev_handle_t *handle, int slot, uint8_t dir, int remote_slot)
{
    bb_get_mcs_in_t input;
    bb_get_mcs_out_t output;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.slot = (uint8_t)slot;
    input.dir = dir;
    ret = link_monitor_ioctl(handle,
                             BB_GET_MCS,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_MCS %s remote slot=%d failed, ret=%d\n", dir_name(dir), remote_slot, ret);
        } else {
            printf("BB_GET_MCS %s failed, ret=%d\n", dir_name(dir), ret);
        }
        return ret;
    }

    printf("  %s: mcs_raw=%u mcs_real=%d theory_throughput=%u kbps\n",
           dir_name(dir),
           output.mcs,
           (int)output.mcs - 2,
           output.throughput);
    return 0;
}

static int query_mcs(bb_dev_handle_t *handle, int slot, int remote_slot)
{
    int ret;

    print_query_title("BB_GET_MCS", remote_slot);
    printf("slot=%d\n", slot);
    ret = query_mcs_dir(handle, slot, BB_DIR_TX, remote_slot);
    if (ret) {
        return ret;
    }

    return query_mcs_dir(handle, slot, BB_DIR_RX, remote_slot);
}

static int query_mcs_mode(bb_dev_handle_t *handle, int slot, int remote_slot)
{
    bb_get_mcs_mode_in_t input;
    bb_get_mcs_mode_out_t output;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.slot = (uint8_t)slot;
    ret = link_monitor_ioctl(handle,
                             BB_GET_MCS_MODE,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_MCS_MODE remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_MCS_MODE failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_MCS_MODE", remote_slot);
    printf("slot=%d auto_mode=%s(%u)\n",
           slot,
           auto_mode_name(output.auto_mode),
           output.auto_mode);
    return 0;
}

static int query_bandwidth_mode(bb_dev_handle_t *handle, int slot, int remote_slot)
{
    bb_get_bandwidth_mode_in_t input;
    bb_get_bandwidth_mode_out_t output;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.slot = (uint8_t)slot;
    ret = link_monitor_ioctl(handle,
                             BB_GET_BANDWIDTH_MODE,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_BANDWIDTH_MODE remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_BANDWIDTH_MODE failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_BANDWIDTH_MODE", remote_slot);
    printf("slot=%d auto_mode=%s(%u)\n",
           slot,
           auto_mode_name(output.auto_mode),
           output.auto_mode);
    return 0;
}

static int query_cur_power(bb_dev_handle_t *handle, int user, int remote_slot)
{
    bb_get_cur_pwr_in_t input;
    bb_get_cur_pwr_out_t output;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.usr = (uint8_t)user;
    ret = link_monitor_ioctl(handle,
                             BB_GET_CUR_POWER,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_CUR_POWER remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_CUR_POWER failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_CUR_POWER", remote_slot);
    printf("user[%u]: power=%u\n", output.usr, output.pwr);
    return 0;
}

static int query_chan_info(bb_dev_handle_t *handle, int remote_slot)
{
    bb_get_chan_info_out_t output;
    int chan_num;
    int ret;

    memset(&output, 0, sizeof(output));
    ret = link_monitor_ioctl(handle,
                             BB_GET_CHAN_INFO,
                             NULL,
                             0,
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_CHAN_INFO remote slot=%d failed, ret=%d\n", remote_slot, ret);
        }
        else {
            printf("BB_GET_CHAN_INFO failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_CHAN_INFO", remote_slot);
    printf("chan_num=%u auto_mode=%u acs_chan=%u work_chan=%u\n",
           output.chan_num,
           output.auto_mode,
           output.acs_chan,
           output.work_chan);

    chan_num = output.chan_num;
    if (chan_num > BB_CONFIG_MAX_CHAN_NUM) {
        printf("chan_num exceeds max, clamp to %d\n", BB_CONFIG_MAX_CHAN_NUM);
        chan_num = BB_CONFIG_MAX_CHAN_NUM;
    }

    for (int i = 0; i < chan_num; ++i) {
        printf("  chan[%d]: freq=%u KHz power=%d dbm\n", i, output.freq[i], output.power[i]);
    }
    return 0;
}

static int query_band_info(bb_dev_handle_t *handle, int remote_slot)
{
    bb_get_band_info_in_t input;
    bb_get_band_info_out_t output;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    ret = link_monitor_ioctl(handle,
                             BB_GET_BAND_INFO,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_BAND_INFO remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_BAND_INFO failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_BAND_INFO", remote_slot);
    printf("band_mode=%s(%u) work_band=%s(%u)\n",
           auto_mode_name(output.band_mode),
           output.band_mode,
           band_name(output.work_band),
           output.work_band);
    return 0;
}

static int query_throughput(bb_dev_handle_t *handle, int slot, int remote_slot)
{
    bb_get_throughput_in_t input;
    bb_get_throughput_out_t output;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.slot = (uint8_t)slot;
    input.dir_bmp = (uint8_t)((1u << BB_DIR_TX) | (1u << BB_DIR_RX));
    ret = link_monitor_ioctl(handle,
                             BB_GET_THROUGHPUT,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_THROUGHPUT remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_THROUGHPUT failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_THROUGHPUT", remote_slot);
    printf("slot=%d\n", slot);
    for (int dir = 0; dir < BB_DIR_MAX; ++dir) {
        if ((input.dir_bmp & (1u << dir)) == 0) {
            continue;
        }

        printf("  %s: phy=%u real=%u\n",
               dir_name((uint8_t)dir),
               output.throughput[dir].phy_throughput,
               output.throughput[dir].real_throughput);
    }
    return 0;
}

static int query_distc_result(bb_dev_handle_t *handle, int slot, int remote_slot)
{
    bb_get_distc_result_in_t input;
    bb_get_distc_result_out_t output;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.slot_bmp = (uint8_t)(1u << slot);
    ret = link_monitor_ioctl(handle,
                             BB_GET_DISTC_RESULT,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_DISTC_RESULT remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_DISTC_RESULT failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_DISTC_RESULT", remote_slot);
    printf("slot=%d\n", slot);
    printf("  slot[%d]: distance=%d\n", slot, output.distance[slot]);
    return 0;
}

static int query_1v1_info(bb_dev_handle_t *handle, int remote_slot)
{
    bb_get_1v1_info_in_t input;
    bb_get_1v1_info_out_t output;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.frame_num = 0;
    ret = link_monitor_ioctl(handle,
                             BB_GET_1V1_INFO,
                             &input,
                             sizeof(input),
                             &output,
                             sizeof(output),
                             remote_slot);
    if (ret) {
        if (remote_slot >= 0) {
            printf("BB_GET_1V1_INFO remote slot=%d failed, ret=%d\n", remote_slot, ret);
        } else {
            printf("BB_GET_1V1_INFO failed, ret=%d\n", ret);
        }
        return ret;
    }

    print_query_title("BB_GET_1V1_INFO", remote_slot);
    print_1v1_info("self", &output.self);
    print_1v1_info("peer", &output.peer);
    return 0;
}

int main(int argc, char **argv)
{
    const char *addr = "127.0.0.1";
    int port = BB_PORT_DEFAULT;
    int dev_index = 0;
    int slot = 0;
    int user = 0;
    int user_specified = 0;
    int do_status = 0;
    int do_user_quality = 0;
    int do_peer_quality = 0;
    int do_mcs = 0;
    int do_mcs_mode = 0;
    int do_bandwidth_mode = 0;
    int do_power = 0;
    int do_chan = 0;
    int do_band = 0;
    int do_throughput = 0;
    int do_distc_result = 0;
    int do_1v1_info = 0;
    int remote = 0;
    int opt;
    int ret = 0;
    int need_link_detail;
    int link_ready = 1;
    bb_demo_context_t ctx;
    bb_get_status_out_t status;

    while ((opt = getopt(argc, argv, "ha:p:i:s:u:ASQqMmWPCBTDVR")) != -1) {
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
        case 's':
            slot = (int)strtol(optarg, NULL, 10);
            break;
        case 'u':
            user = (int)strtol(optarg, NULL, 10);
            user_specified = 1;
            break;
        case 'A':
            do_status = 1;
            do_user_quality = 1;
            do_peer_quality = 1;
            do_mcs = 1;
            do_mcs_mode = 1;
            do_bandwidth_mode = 1;
            do_power = 1;
            do_chan = 1;
            do_band = 1;
            do_throughput = 1;
            do_distc_result = 1;
            do_1v1_info = 1;
            break;
        case 'S':
            do_status = 1;
            break;
        case 'Q':
            do_user_quality = 1;
            break;
        case 'q':
            do_peer_quality = 1;
            break;
        case 'M':
            do_mcs = 1;
            break;
        case 'm':
            do_mcs_mode = 1;
            break;
        case 'W':
            do_bandwidth_mode = 1;
            break;
        case 'P':
            do_power = 1;
            break;
        case 'C':
            do_chan = 1;
            break;
        case 'R':
            remote = 1;
            break;
        case 'B':
            do_band = 1;
            break;
        case 'T':
            do_throughput = 1;
            break;
        case 'D':
            do_distc_result = 1;
            break;
        case 'V':
            do_1v1_info = 1;
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    if (!do_status && !do_user_quality && !do_peer_quality && !do_mcs &&
        !do_mcs_mode && !do_bandwidth_mode && !do_power && !do_chan && !do_band &&
        !do_throughput && !do_distc_result && !do_1v1_info) {
        do_status = 1;
        do_user_quality = 1;
        do_peer_quality = 1;
        do_mcs = 1;
        do_mcs_mode = 1;
        do_bandwidth_mode = 1;
        do_power = 1;
        do_chan = 1;
        do_band = 1;
        do_throughput = 1;
        do_distc_result = 1;
        do_1v1_info = 1;
    }

    if (slot < 0 || slot >= BB_SLOT_MAX) {
        printf("invalid slot %d, valid range: 0-%d\n", slot, BB_SLOT_MAX - 1);
        return -1;
    }

    if (user < 0 || user >= BB_DATA_USER_MAX) {
        printf("invalid user %d, valid range: 0-%d\n", user, BB_DATA_USER_MAX - 1);
        return -1;
    }

    ret = bb_demo_open(&ctx, addr, port, dev_index);
    if (ret) {
        return -1;
    }

    need_link_detail = do_user_quality || do_peer_quality || do_mcs || do_power ||
                       do_throughput || do_distc_result;
    if (do_status || need_link_detail) {
        ret = read_status(ctx.handle, &status, remote ? slot : -1);
        if (ret) {
            goto done;
        }

        if (do_status) {
            print_status(&status, remote ? slot : -1);
        }

        if (need_link_detail) {
            link_ready = link_ready_for_slot(&status, slot);
            if (!link_ready) {
                if (!do_status) {
                    print_status(&status, remote ? slot : -1);
                }

                printf("\nslot[%d] is not paired/connected, skip link detail queries\n", slot);
                printf("link detail APIs require pair_state=1 or state=CONNECT\n");
            }
        }
    }

    if (do_user_quality && link_ready) {
        int quality_user = user_specified ? user : default_quality_user_for_role(status.role);
        ret = query_user_quality(ctx.handle, quality_user, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_peer_quality && link_ready) {
        ret = query_peer_quality(ctx.handle, slot, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_mcs && link_ready) {
        ret = query_mcs(ctx.handle, slot, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_mcs_mode) {
        ret = query_mcs_mode(ctx.handle, slot, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_bandwidth_mode) {
        ret = query_bandwidth_mode(ctx.handle, slot, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_power && link_ready) {
        int power_user = user_specified ? user : default_power_user_for_role(status.role);
        ret = query_cur_power(ctx.handle, power_user, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_chan) {
        ret = query_chan_info(ctx.handle, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_band) {
        ret = query_band_info(ctx.handle, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_throughput && link_ready) {
        ret = query_throughput(ctx.handle, slot, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_distc_result && link_ready) {
        ret = query_distc_result(ctx.handle, slot, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    if (do_1v1_info) {
        ret = query_1v1_info(ctx.handle, remote ? slot : -1);
        if (ret) {
            goto done;
        }
    }

    printf("\nl4 link monitor finished\n");

done:
    bb_demo_close(&ctx);
    return ret ? -1 : 0;
}

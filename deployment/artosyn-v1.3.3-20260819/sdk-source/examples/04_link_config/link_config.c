#include "bb_demo_common.h"

#include "getopt.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LINK_CONFIG_CMD_INTERVAL_US (200 * 1000)

static void usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  -h              show this help\n");
    printf("  -a <addr>       daemon address, default: 127.0.0.1\n");
    printf("  -p <port>       daemon port, default: %d\n", BB_PORT_DEFAULT);
    printf("  -i <index>      device index, default: 0\n");
    printf("  -s <slot>       slot id, default: 0; remote slot id for -R\n");
    printf("  -u <user>       physical user id for -P, default: AP=%d, DEV=%d\n",
           BB_USER_BR_CS,
           BB_USER_0);
    printf("  -R              set peer device by remote ioctl through -s <slot>\n");
    printf("  -B <0|1>        set BB_SET_BAND_MODE, 1=auto, 0=manual\n");
    printf("  -b <band>       set BB_SET_BAND, band: 1g/2g/5g or 0/1/2\n");
    printf("  -C <0|1>        set BB_SET_CHAN_MODE, 1=auto, 0=manual\n");
    printf("  -c <index>      set BB_SET_CHAN channel index, range: 0-255\n");
    printf("  -d <dir>        direction for -c/-w, tx/rx or 0/1, default: rx\n");
    printf("  -W <0|1>        set BB_SET_BANDWIDTH_MODE, 1=auto, 0=manual\n");
    printf("  -w <bandwidth>  set BB_SET_BANDWIDTH, 0-5: 1.25M/2.5M/5M/10M/20M/40M\n");
    printf("  -M <0|1>        set BB_SET_MCS_MODE, 1=auto, 0=manual\n");
    printf("  -m <mcs>        set BB_SET_MCS, range: 0-%d\n", BB_PHY_MCS_MAX - 1);
    printf("  -O <0|1>        set BB_SET_POWER_AUTO, 1=auto, 0=manual\n");
    printf("  -P <power>      set BB_SET_POWER, range: 0-31 dBm\n");
    printf("  -F <0|1>        set BB_SET_FRAME_CHANGE, AP only and only valid in SINGLE_USER mode\n");
}

static int str_eq_ignore_case(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }

        ++a;
        ++b;
    }

    return *a == '\0' && *b == '\0';
}

static int parse_int_range(const char *text, int min_value, int max_value, const char *name, int *out)
{
    char *end = NULL;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);
    if (errno || end == text || *end != '\0' || value < min_value || value > max_value) {
        printf("invalid %s: %s, valid range: %d-%d\n", name, text, min_value, max_value);
        return -1;
    }

    *out = (int)value;
    return 0;
}

static int parse_auto_mode(const char *text, const char *name, int *out)
{
    return parse_int_range(text, 0, 1, name, out);
}

static int parse_dir(const char *text, int *out)
{
    if (str_eq_ignore_case(text, "tx")) {
        *out = BB_DIR_TX;
        return 0;
    }

    if (str_eq_ignore_case(text, "rx")) {
        *out = BB_DIR_RX;
        return 0;
    }

    return parse_int_range(text, BB_DIR_TX, BB_DIR_RX, "dir", out);
}

static int parse_band(const char *text, int *out)
{
    if (str_eq_ignore_case(text, "1g") || str_eq_ignore_case(text, "1.4g") ||
        str_eq_ignore_case(text, "14g")) {
        *out = BB_BAND_1G;
        return 0;
    }

    if (str_eq_ignore_case(text, "2g")) {
        *out = BB_BAND_2G;
        return 0;
    }

    if (str_eq_ignore_case(text, "5g")) {
        *out = BB_BAND_5G;
        return 0;
    }

    return parse_int_range(text, BB_BAND_1G, BB_BAND_5G, "band", out);
}

static const char *dir_name(int dir)
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

static const char *band_name(int band)
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

static const char *bandwidth_name(int bandwidth)
{
    switch (bandwidth) {
    case BB_BW_1_25M:
        return "1.25M";
    case BB_BW_2_5M:
        return "2.5M";
    case BB_BW_5M:
        return "5M";
    case BB_BW_10M:
        return "10M";
    case BB_BW_20M:
        return "20M";
    case BB_BW_40M:
        return "40M";
    default:
        return "UNKNOWN";
    }
}

static const char *mode_name(int mode)
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

static const char *role_name(int role)
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

static int link_config_ioctl(bb_dev_handle_t *handle,
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

static int read_status(bb_dev_handle_t *handle, bb_get_status_out_t *status, int remote_slot)
{
    bb_get_status_in_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    memset(status, 0, sizeof(*status));

    input.user_bmp = 0xffff;
    ret = link_config_ioctl(handle,
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

static void print_config_title(const char *title, int remote_slot)
{
    if (remote_slot >= 0) {
        printf("\n[%s remote slot=%d]\n", title, remote_slot);
        return;
    }

    printf("\n[%s]\n", title);
}

static void delay_before_config_cmd(int executed_count)
{
    if (executed_count > 0) {
        usleep(LINK_CONFIG_CMD_INTERVAL_US);
    }
}

static int set_band_mode(bb_dev_handle_t *handle, int auto_mode, int remote_slot)
{
    bb_set_band_mode_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.auto_mode = (uint8_t)auto_mode;

    ret = link_config_ioctl(handle,
                            BB_SET_BAND_MODE,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_BAND_MODE failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_BAND_MODE", remote_slot);
    printf("auto_mode=%u\n", input.auto_mode);
    return 0;
}

static int set_band(bb_dev_handle_t *handle, int target_band, int remote_slot)
{
    bb_set_band_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.target_band = (uint8_t)target_band;

    ret = link_config_ioctl(handle,
                            BB_SET_BAND,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_BAND failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_BAND", remote_slot);
    printf("target_band=%s(%u)\n", band_name(input.target_band), input.target_band);
    return 0;
}

static int set_chan_mode(bb_dev_handle_t *handle, int auto_mode, int remote_slot)
{
    bb_set_chan_mode_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.auto_mode = (uint8_t)auto_mode;

    ret = link_config_ioctl(handle,
                            BB_SET_CHAN_MODE,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_CHAN_MODE failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_CHAN_MODE", remote_slot);
    printf("auto_mode=%u\n", input.auto_mode);
    return 0;
}

static int set_chan(bb_dev_handle_t *handle, int chan_dir, int chan_index, int remote_slot)
{
    bb_set_chan_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.chan_dir = (uint8_t)chan_dir;
    input.chan_index = (uint8_t)chan_index;

    ret = link_config_ioctl(handle,
                            BB_SET_CHAN,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_CHAN failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_CHAN", remote_slot);
    printf("dir=%s(%u) chan_index=%u\n",
           dir_name(input.chan_dir),
           input.chan_dir,
           input.chan_index);
    return 0;
}

static int set_bandwidth_mode(bb_dev_handle_t *handle, int slot, int mode, int remote_slot)
{
    bb_set_bandwidth_mode_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.slot = (uint8_t)slot;
    input.mode = (uint8_t)mode;

    ret = link_config_ioctl(handle,
                            BB_SET_BANDWIDTH_MODE,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_BANDWIDTH_MODE failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_BANDWIDTH_MODE", remote_slot);
    printf("slot=%u mode=%u\n", input.slot, input.mode);
    return 0;
}

static int set_bandwidth(bb_dev_handle_t *handle, int slot, int dir, int bandwidth, int remote_slot)
{
    bb_set_bandwidth_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.slot = (uint8_t)slot;
    input.dir = (uint8_t)dir;
    input.bandwidth = (uint8_t)bandwidth;

    ret = link_config_ioctl(handle,
                            BB_SET_BANDWIDTH,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_BANDWIDTH failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_BANDWIDTH", remote_slot);
    printf("slot=%u dir=%s(%u) bandwidth=%s(%u)\n",
           input.slot,
           dir_name(input.dir),
           input.dir,
           bandwidth_name(input.bandwidth),
           input.bandwidth);
    return 0;
}

static int set_mcs_mode(bb_dev_handle_t *handle, int slot, int auto_mode, int remote_slot)
{
    bb_set_mcs_mode_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.slot = (uint8_t)slot;
    input.auto_mode = (uint8_t)auto_mode;

    ret = link_config_ioctl(handle,
                            BB_SET_MCS_MODE,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_MCS_MODE failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_MCS_MODE", remote_slot);
    printf("slot=%u auto_mode=%u\n",
           input.slot,
           input.auto_mode);
    return 0;
}

static int set_mcs(bb_dev_handle_t *handle, int slot, int mcs, int remote_slot)
{
    bb_set_mcs_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.slot = (uint8_t)slot;
    input.mcs = (uint8_t)mcs;

    ret = link_config_ioctl(handle,
                            BB_SET_MCS,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_MCS failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_MCS", remote_slot);
    printf("slot=%u mcs=%u\n", input.slot, input.mcs);
    return 0;
}

static int resolve_power_user(bb_dev_handle_t *handle, int remote_slot, int *user)
{
    bb_get_status_out_t status;
    int ret;

    ret = read_status(handle, &status, remote_slot);
    if (ret) {
        return ret;
    }

    *user = default_power_user_for_role(status.role);
    return 0;
}

static int set_power_auto(bb_dev_handle_t *handle, int pwr_auto, int remote_slot)
{
    bb_set_pwr_auto_in_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.pwr_auto = (uint8_t)pwr_auto;

    ret = link_config_ioctl(handle,
                            BB_SET_POWER_AUTO,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_POWER_AUTO failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_POWER_AUTO", remote_slot);
    printf("pwr_auto=%u\n", input.pwr_auto);
    return 0;
}

static int set_power(bb_dev_handle_t *handle, int user, int power, int remote_slot)
{
    bb_set_pwr_in_t input;
    int ret;

    memset(&input, 0, sizeof(input));
    input.usr = (uint8_t)user;
    input.pwr = (uint8_t)power;

    ret = link_config_ioctl(handle,
                            BB_SET_POWER,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_POWER failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_POWER", remote_slot);
    printf("user=%u power=%u dBm\n", input.usr, input.pwr);
    return 0;
}

static int set_frame_change(bb_dev_handle_t *handle, int mode, int remote_slot)
{
    bb_get_status_out_t status;
    bb_set_frame_change_t input;
    int ret;

    ret = read_status(handle, &status, remote_slot);
    if (ret) {
        return ret;
    }

    if (status.role != BB_ROLE_AP) {
        print_config_title("BB_SET_FRAME_CHANGE", remote_slot);
        printf("requires AP role, current role=%s(%u)\n",
               role_name(status.role),
               status.role);
        return -1;
    }

    if (status.mode != BB_MODE_SINGLE_USER) {
        print_config_title("BB_SET_FRAME_CHANGE", remote_slot);
        printf("requires SINGLE_USER mode, current mode=%s(%u)\n",
               mode_name(status.mode),
               status.mode);
        return -1;
    }

    memset(&input, 0, sizeof(input));
    input.mode = (uint8_t)mode;

    ret = link_config_ioctl(handle,
                            BB_SET_FRAME_CHANGE,
                            &input,
                            sizeof(input),
                            NULL,
                            0,
                            remote_slot);
    if (ret) {
        printf("BB_SET_FRAME_CHANGE failed, ret=%d\n", ret);
        return ret;
    }

    print_config_title("BB_SET_FRAME_CHANGE", remote_slot);
    printf("mode=%u\n", input.mode);
    return 0;
}

int main(int argc, char **argv)
{
    const char *addr = "127.0.0.1";
    int port = BB_PORT_DEFAULT;
    int dev_index = 0;
    int slot = 0;
    int chan_mode = 0;
    int chan_index = 0;
    int chan_dir = BB_DIR_RX;
    int band_mode = 0;
    int target_band = 0;
    int bandwidth_mode = 0;
    int bandwidth = 0;
    int mcs_mode = 0;
    int mcs = 0;
    int power_user = 0;
    int power_auto = 0;
    int power = 0;
    int frame_change_mode = 0;
    int remote = 0;
    int power_user_specified = 0;
    int do_chan_mode = 0;
    int do_chan = 0;
    int do_band_mode = 0;
    int do_band = 0;
    int do_bandwidth_mode = 0;
    int do_bandwidth = 0;
    int do_mcs_mode = 0;
    int do_mcs = 0;
    int do_power_auto = 0;
    int do_power = 0;
    int do_frame_change = 0;
    int executed_count = 0;
    int opt;
    int ret = 0;
    bb_demo_context_t ctx;

    while ((opt = getopt(argc, argv, "ha:p:i:s:u:RC:c:d:B:b:W:w:M:m:O:P:F:")) != -1) {
        switch (opt) {
        case 'h':
            usage(argv[0]);
            return 0;
        case 'a':
            addr = optarg;
            break;
        case 'p':
            if (parse_int_range(optarg, 1, 65535, "port", &port)) {
                return -1;
            }
            break;
        case 'i':
            if (parse_int_range(optarg, 0, 255, "device index", &dev_index)) {
                return -1;
            }
            break;
        case 's':
            if (parse_int_range(optarg, 0, BB_SLOT_MAX - 1, "slot", &slot)) {
                return -1;
            }
            break;
        case 'u':
            if (parse_int_range(optarg, 0, BB_USER_MAX - 1, "user", &power_user)) {
                return -1;
            }
            power_user_specified = 1;
            break;
        case 'R':
            remote = 1;
            break;
        case 'C':
            if (parse_auto_mode(optarg, "chan auto_mode", &chan_mode)) {
                return -1;
            }
            do_chan_mode = 1;
            break;
        case 'c':
            if (parse_int_range(optarg, 0, 255, "channel index", &chan_index)) {
                return -1;
            }
            do_chan = 1;
            break;
        case 'd':
            if (parse_dir(optarg, &chan_dir)) {
                return -1;
            }
            break;
        case 'B':
            if (parse_auto_mode(optarg, "band auto_mode", &band_mode)) {
                return -1;
            }
            do_band_mode = 1;
            break;
        case 'b':
            if (parse_band(optarg, &target_band)) {
                return -1;
            }
            do_band = 1;
            break;
        case 'W':
            if (parse_auto_mode(optarg, "bandwidth mode", &bandwidth_mode)) {
                return -1;
            }
            do_bandwidth_mode = 1;
            break;
        case 'w':
            if (parse_int_range(optarg, 0, BB_BW_MAX - 1, "bandwidth", &bandwidth)) {
                return -1;
            }
            do_bandwidth = 1;
            break;
        case 'M':
            if (parse_auto_mode(optarg, "mcs auto_mode", &mcs_mode)) {
                return -1;
            }
            do_mcs_mode = 1;
            break;
        case 'm':
            if (parse_int_range(optarg, 0, BB_PHY_MCS_MAX - 1, "mcs", &mcs)) {
                return -1;
            }
            do_mcs = 1;
            break;
        case 'O':
            if (parse_auto_mode(optarg, "power auto", &power_auto)) {
                return -1;
            }
            do_power_auto = 1;
            break;
        case 'P':
            if (parse_int_range(optarg, 0, 31, "power", &power)) {
                return -1;
            }
            do_power = 1;
            break;
        case 'F':
            if (parse_int_range(optarg, 0, 1, "frame change mode", &frame_change_mode)) {
                return -1;
            }
            do_frame_change = 1;
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    if (!do_chan_mode && !do_chan && !do_band_mode && !do_band &&
        !do_bandwidth_mode && !do_bandwidth && !do_mcs_mode && !do_mcs &&
        !do_power_auto && !do_power && !do_frame_change) {
        printf("no link config action specified\n");
        usage(argv[0]);
        return -1;
    }

    ret = bb_demo_open(&ctx, addr, port, dev_index);
    if (ret) {
        return -1;
    }

    if (do_band_mode) {
        delay_before_config_cmd(executed_count);
        ret = set_band_mode(ctx.handle, band_mode, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_band) {
        delay_before_config_cmd(executed_count);
        ret = set_band(ctx.handle, target_band, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_chan_mode) {
        delay_before_config_cmd(executed_count);
        ret = set_chan_mode(ctx.handle, chan_mode, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_chan) {
        delay_before_config_cmd(executed_count);
        ret = set_chan(ctx.handle, chan_dir, chan_index, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_bandwidth_mode) {
        delay_before_config_cmd(executed_count);
        ret = set_bandwidth_mode(ctx.handle, slot, bandwidth_mode, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_bandwidth) {
        delay_before_config_cmd(executed_count);
        ret = set_bandwidth(ctx.handle, slot, chan_dir, bandwidth, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_mcs_mode) {
        delay_before_config_cmd(executed_count);
        ret = set_mcs_mode(ctx.handle, slot, mcs_mode, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_mcs) {
        delay_before_config_cmd(executed_count);
        ret = set_mcs(ctx.handle, slot, mcs, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_power_auto) {
        delay_before_config_cmd(executed_count);
        ret = set_power_auto(ctx.handle, power_auto, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_power) {
        if (!power_user_specified) {
            ret = resolve_power_user(ctx.handle, remote ? slot : -1, &power_user);
            if (ret) {
                goto done;
            }
        }

        delay_before_config_cmd(executed_count);
        ret = set_power(ctx.handle, power_user, power, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    if (do_frame_change) {
        delay_before_config_cmd(executed_count);
        ret = set_frame_change(ctx.handle, frame_change_mode, remote ? slot : -1);
        if (ret) {
            goto done;
        }
        ++executed_count;
    }

    printf("\nl4 link config finished\n");

done:
    bb_demo_close(&ctx);
    return ret ? -1 : 0;
}

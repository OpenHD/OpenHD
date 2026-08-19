#include "bb_demo_common.h"

#include "getopt.h"
#include "prj_rpc.h"
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ACTION_NONE = 0,
    ACTION_GET_UART,
    ACTION_SET_UART,
} action_t;

typedef struct {
    const char *addr;
    int port;
    int dev_index;
    action_t action;
    int ioctl_slot;
    int uart_id;
    int set_param_seen;
    uint32_t baudrate;
    uint8_t dbit;
    uint8_t parity;
    uint8_t stop_bit;
    uint32_t rx_buff_size;
    uint8_t apply;
} options_t;

static void usage(const char *prog)
{
    printf("Usage: %s [options] <one action>\n", prog);
    printf("\n");
    printf("Common options:\n");
    printf("  -h, --help              show this help\n");
    printf("  -a, --addr <addr>       daemon address, default: 127.0.0.1\n");
    printf("  -p, --port <port>       daemon port, default: %d\n", BB_PORT_DEFAULT);
    printf("  -i, --index <index>     device index, default: 0\n");
    printf("\n");
    printf("Query actions:\n");
    printf("  -g, --get-uart <id>     get running UART config\n");
    printf("\n");
    printf("Set actions:\n");
    printf("  -U, --set-uart <id>     set UART config\n");
    printf("  -b, --baudrate <rate>   UART baudrate, default: 115200\n");
    printf("  -d, --data-bit <5-8>    UART data bits, default: 8\n");
    printf("  -P, --parity <parity>   parity: none/even/odd/0/1/2, default: none\n");
    printf("  -T, --stop-bit <1-3>    stop bit protocol value, default: 1\n");
    printf("  -r, --rx-buf-size <n>   RX buffer size, default: 0\n");
    printf("  -A, --apply             apply to running system\n");
    printf("\n");
    printf("Other options:\n");
    printf("  -s, --slot <slot>       remote ioctl slot id\n");
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

static void init_options(options_t *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->addr = "127.0.0.1";
    opts->port = BB_PORT_DEFAULT;
    opts->dev_index = 0;
    opts->action = ACTION_NONE;
    opts->ioctl_slot = -1;
    opts->uart_id = -1;
    opts->baudrate = 115200;
    opts->dbit = 8;
    opts->parity = 0;
    opts->stop_bit = 1;
    opts->rx_buff_size = 0;
}

static int parse_int_range(const char *text, int min_value, int max_value, const char *name, int *out)
{
    char *end = NULL;
    long value;

    errno = 0;
    value = strtol(text, &end, 0);
    if (errno || end == text || *end != '\0' || value < min_value || value > max_value) {
        printf("invalid %s: %s, valid range: %d-%d\n", name, text, min_value, max_value);
        return -1;
    }

    *out = (int)value;
    return 0;
}

static int parse_u32_range(const char *text,
                           uint32_t min_value,
                           uint32_t max_value,
                           const char *name,
                           uint32_t *out)
{
    char *end = NULL;
    unsigned long value;

    errno = 0;
    value = strtoul(text, &end, 0);
    if (errno || end == text || *end != '\0' || value < min_value || value > max_value) {
        printf("invalid %s: %s, valid range: %u-%u\n", name, text, min_value, max_value);
        return -1;
    }

    *out = (uint32_t)value;
    return 0;
}

static int parse_parity(const char *text, uint8_t *out)
{
    int value;

    if (str_eq_ignore_case(text, "none")) {
        *out = 0;
        return 0;
    }

    if (str_eq_ignore_case(text, "even")) {
        *out = 1;
        return 0;
    }

    if (str_eq_ignore_case(text, "odd")) {
        *out = 2;
        return 0;
    }

    if (parse_int_range(text, 0, 2, "parity", &value)) {
        return -1;
    }

    *out = (uint8_t)value;
    return 0;
}

static int set_action(options_t *opts, action_t action, const char *uart_id_text)
{
    int uart_id;

    if (opts->action != ACTION_NONE) {
        printf("only one action can be specified\n");
        return -1;
    }

    if (parse_int_range(uart_id_text, 1, 255, "uart id", &uart_id)) {
        return -1;
    }

    opts->action = action;
    opts->uart_id = uart_id;
    return 0;
}

static int parse_options(int argc, char **argv, options_t *opts)
{
    enum {
        OPT_GET_UART = 1000,
        OPT_SET_UART,
        OPT_BAUDRATE,
        OPT_DATA_BIT,
        OPT_PARITY,
        OPT_STOP_BIT,
        OPT_RX_BUF_SIZE,
        OPT_APPLY,
        OPT_SLOT,
        OPT_ADDR,
        OPT_PORT,
        OPT_INDEX,
        OPT_HELP,
    };

    static const struct option long_options[] = {
        {"get-uart", required_argument, NULL, OPT_GET_UART},
        {"set-uart", required_argument, NULL, OPT_SET_UART},
        {"baudrate", required_argument, NULL, OPT_BAUDRATE},
        {"data-bit", required_argument, NULL, OPT_DATA_BIT},
        {"parity", required_argument, NULL, OPT_PARITY},
        {"stop-bit", required_argument, NULL, OPT_STOP_BIT},
        {"rx-buf-size", required_argument, NULL, OPT_RX_BUF_SIZE},
        {"apply", no_argument, NULL, OPT_APPLY},
        {"slot", required_argument, NULL, OPT_SLOT},
        {"addr", required_argument, NULL, OPT_ADDR},
        {"port", required_argument, NULL, OPT_PORT},
        {"index", required_argument, NULL, OPT_INDEX},
        {"help", no_argument, NULL, OPT_HELP},
        {NULL, 0, NULL, 0},
    };

    int opt;

    init_options(opts);

    while ((opt = getopt_long(argc, argv, "ha:p:i:g:U:b:d:P:T:r:As:", long_options, NULL)) != -1) {
        int int_value;
        uint32_t u32_value;

        switch (opt) {
        case 'h':
        case OPT_HELP:
            usage(argv[0]);
            return 1;
        case 'a':
        case OPT_ADDR:
            opts->addr = optarg;
            break;
        case 'p':
        case OPT_PORT:
            if (parse_int_range(optarg, 1, 65535, "port", &opts->port)) {
                return -1;
            }
            break;
        case 'i':
        case OPT_INDEX:
            if (parse_int_range(optarg, 0, 255, "index", &opts->dev_index)) {
                return -1;
            }
            break;
        case 'g':
        case OPT_GET_UART:
            if (set_action(opts, ACTION_GET_UART, optarg)) {
                return -1;
            }
            break;
        case 'U':
        case OPT_SET_UART:
            if (set_action(opts, ACTION_SET_UART, optarg)) {
                return -1;
            }
            break;
        case 'b':
        case OPT_BAUDRATE:
            if (parse_u32_range(optarg, 1, UINT32_MAX, "baudrate", &opts->baudrate)) {
                return -1;
            }
            opts->set_param_seen = 1;
            break;
        case 'd':
        case OPT_DATA_BIT:
            if (parse_int_range(optarg, 5, 8, "data bit", &int_value)) {
                return -1;
            }
            opts->dbit = (uint8_t)int_value;
            opts->set_param_seen = 1;
            break;
        case 'P':
        case OPT_PARITY:
            if (parse_parity(optarg, &opts->parity)) {
                return -1;
            }
            opts->set_param_seen = 1;
            break;
        case 'T':
        case OPT_STOP_BIT:
            if (parse_int_range(optarg, 1, 3, "stop bit", &int_value)) {
                return -1;
            }
            opts->stop_bit = (uint8_t)int_value;
            opts->set_param_seen = 1;
            break;
        case 'r':
        case OPT_RX_BUF_SIZE:
            if (parse_u32_range(optarg, 0, UINT32_MAX, "rx buffer size", &u32_value)) {
                return -1;
            }
            opts->rx_buff_size = u32_value;
            opts->set_param_seen = 1;
            break;
        case 'A':
        case OPT_APPLY:
            opts->apply = 1;
            opts->set_param_seen = 1;
            break;
        case 's':
        case OPT_SLOT:
            if (parse_int_range(optarg, 0, 255, "slot", &opts->ioctl_slot)) {
                return -1;
            }
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    if (optind < argc) {
        printf("unexpected argument: %s\n", argv[optind]);
        return -1;
    }

    if (opts->action == ACTION_NONE) {
        usage(argv[0]);
        return -1;
    }

    if (opts->action == ACTION_GET_UART && opts->set_param_seen) {
        printf("UART set options can only be used with --set-uart\n");
        return -1;
    }

    return 0;
}

static int ar8030_ioctl(bb_dev_handle_t *handle,
                        uint32_t request,
                        const void *in,
                        size_t in_len,
                        void *out,
                        size_t out_len,
                        int ioctl_slot)
{
    bb_remote_ioctl_in_t remote_in;
    bb_remote_ioctl_out_t remote_out;
    int ret;

    if (ioctl_slot < 0) {
        return bb_ioctl(handle, request, in, out);
    }

    if (in_len > sizeof(remote_in.data) || out_len > sizeof(remote_out.data)) {
        printf("remote ioctl payload too large\n");
        return -1;
    }

    memset(&remote_in, 0, sizeof(remote_in));
    memset(&remote_out, 0, sizeof(remote_out));

    if (in && in_len > 0) {
        memcpy(remote_in.data, in, in_len);
    }
    remote_in.len = (uint16_t)in_len;
    remote_in.slot = (uint8_t)ioctl_slot;
    remote_in.msg_id = request;

    ret = bb_ioctl(handle, BB_REMOTE_IOCTL_REQ, &remote_in, &remote_out);
    if (!ret && out && out_len > 0) {
        memcpy(out, remote_out.data, out_len);
    }

    return ret;
}

static const char *parity_name(uint8_t parity)
{
    switch (parity) {
    case 0:
        return "none";
    case 1:
        return "even";
    case 2:
        return "odd";
    default:
        return "unknown";
    }
}

static const char *uart_source_name(int running)
{
    return running ? "running" : "minidb";
}

static void print_uart_config(uint8_t id, const prj_cmd_get_uart_out_t *uart)
{
    printf("id=%u\n", id);
    printf("baudrate=%u\n", uart->baudrate);
    printf("data_bit=%u\n", uart->dbit);
    printf("parity=%s(%u)\n", parity_name(uart->parity), uart->parity);
    printf("stop_bit=%u\n", uart->stop_bit);
    printf("rx_buf_size=%u\n", uart->rx_buff_size);
}

static int set_uart_config(bb_dev_handle_t *handle, const options_t *opts)
{
    bb_set_prj_dispatch_in_t request;
    prj_rpc_hdr_t *hdr;
    prj_cmd_set_uart_t payload;
    int ret;

    memset(&request, 0, sizeof(request));
    memset(&payload, 0, sizeof(payload));

    payload.id = (uint8_t)opts->uart_id;
    payload.apply = opts->apply;
    payload.baudrate = opts->baudrate;
    payload.dbit = opts->dbit;
    payload.parity = opts->parity;
    payload.stop_bit = opts->stop_bit;
    payload.rx_buff_size = opts->rx_buff_size;

    hdr = (prj_rpc_hdr_t *)request.data;
    hdr->cmdid = PRJ_CMD_SET_UART;
    memcpy(hdr->data, &payload, sizeof(payload));

    ret = ar8030_ioctl(handle,
                       BB_SET_PRJ_DISPATCH,
                       &request,
                       sizeof(request),
                       NULL,
                       0,
                       opts->ioctl_slot);
    if (ret) {
        printf("BB_SET_PRJ_DISPATCH cmd=%u failed, ret=%d\n", PRJ_CMD_SET_UART, ret);
        return ret;
    }

    printf("[PRJ_CMD_SET_UART]\n");
    print_uart_config(payload.id, &payload);
    printf("apply=%u\n", payload.apply);
    printf("set uart config ok\n");
    return 0;
}

static int get_uart_config(bb_dev_handle_t *handle, const options_t *opts, int running)
{
    bb_get_prj_dispatch_in_t request;
    bb_get_prj_dispatch_out_t response;
    prj_rpc_hdr_t *hdr;
    prj_rpc_hdr_t *out_hdr;
    prj_cmd_get_uart_in_t input;
    prj_cmd_get_uart_out_t output;
    int ret;

    memset(&request, 0, sizeof(request));
    memset(&response, 0, sizeof(response));
    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.id = (uint8_t)opts->uart_id;
    input.running = running ? 1 : 0;

    hdr = (prj_rpc_hdr_t *)request.data;
    hdr->cmdid = PRJ_CMD_GET_UART;
    memcpy(hdr->data, &input, sizeof(input));

    ret = ar8030_ioctl(handle,
                       BB_GET_PRJ_DISPATCH,
                       &request,
                       sizeof(request),
                       &response,
                       sizeof(response),
                       opts->ioctl_slot);
    if (ret) {
        printf("BB_GET_PRJ_DISPATCH cmd=%u source=%s failed, ret=%d\n",
               PRJ_CMD_GET_UART,
               uart_source_name(running),
               ret);
        return ret;
    }

    out_hdr = (prj_rpc_hdr_t *)response.data;
    memcpy(&output, out_hdr->data, sizeof(output));

    printf("[PRJ_CMD_GET_UART]\n");
    printf("source=%s\n", uart_source_name(running));
    print_uart_config(input.id, &output);
    return 0;
}

static int run_action(bb_dev_handle_t *handle, const options_t *opts)
{
    switch (opts->action) {
    case ACTION_GET_UART:
        return get_uart_config(handle, opts, 1);
    case ACTION_SET_UART:
        return set_uart_config(handle, opts);
    default:
        return -1;
    }
}

int main(int argc, char **argv)
{
    options_t opts;
    bb_demo_context_t ctx;
    int ret;

    ret = parse_options(argc, argv, &opts);
    if (ret > 0) {
        return 0;
    }
    if (ret < 0) {
        return -1;
    }

    ret = bb_demo_open(&ctx, opts.addr, opts.port, opts.dev_index);
    if (ret) {
        return -1;
    }

    ret = run_action(ctx.handle, &opts);

    bb_demo_close(&ctx);
    return ret ? -1 : 0;
}

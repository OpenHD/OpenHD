#include "bb_demo_common.h"

#include "getopt.h"
#include "prj_rpc.h"
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEMO_STATE_DISARMED 0x00
#define DEMO_STATE_ARMED    0xff

typedef enum {
    RUN_MODE_NONE = 0,
    RUN_MODE_RECEIVE,
    RUN_MODE_SEND,
} run_mode_t;

typedef struct {
    uint8_t state;
} receive_context_t;

static volatile sig_atomic_t g_stop_requested;

static const char *state_name(uint8_t state)
{
    return state == DEMO_STATE_ARMED ? "ARMED" : "DISARMED";
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
    printf("  -s <slot>       remote slot for send mode, default: 0\n");
    printf("  -r              receive BB_EVENT_PRJ_DISPATCH events\n");
    printf("  -t <state>      send state to the peer: armed or disarmed\n");
}

static int parse_int_range(const char *text, int min_value, int max_value,
                           const char *name, int *out)
{
    char *end = NULL;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);
    if (errno || end == text || *end != '\0' ||
        value < min_value || value > max_value) {
        printf("invalid %s: %s, valid range: %d-%d\n",
               name, text, min_value, max_value);
        return -1;
    }

    *out = (int)value;
    return 0;
}

static int parse_state(const char *text, uint8_t *state)
{
    if (strcmp(text, "armed") == 0) {
        *state = DEMO_STATE_ARMED;
        return 0;
    }

    if (strcmp(text, "disarmed") == 0) {
        *state = DEMO_STATE_DISARMED;
        return 0;
    }

    printf("invalid state: %s, expected armed or disarmed\n", text);
    return -1;
}

static void stop_signal_handler(int signo)
{
    (void)signo;
    g_stop_requested = 1;
}

static void project_event_cb(void *arg, void *user)
{
    const bb_event_prj_dispatch_t *event = (const bb_event_prj_dispatch_t *)arg;
    receive_context_t *ctx = (receive_context_t *)user;
    const prj_rpc_hdr_t *hdr;
    const prj_cmd_event_demo_t *event_demo;
    uint8_t old_state;

    if (!event || !ctx) {
        printf("received invalid project event callback arguments\n");
        return;
    }

    hdr = (const prj_rpc_hdr_t *)event->data;
    if (hdr->cmdid != PRJ_CMD_EVENT_DEMO) {
        printf("ignore project event cmdid=%u\n", hdr->cmdid);
        return;
    }

    event_demo = (const prj_cmd_event_demo_t *)hdr->data;
    if (event_demo->data != DEMO_STATE_DISARMED &&
        event_demo->data != DEMO_STATE_ARMED) {
        printf("ignore PRJ_CMD_EVENT_DEMO with invalid data=0x%02x\n",
               event_demo->data);
        return;
    }

    old_state = ctx->state;
    ctx->state = event_demo->data;

    printf("[BB_EVENT_PRJ_DISPATCH] cmdid=%u data=0x%02x\n",
           hdr->cmdid, event_demo->data);
    if (old_state == ctx->state) {
        printf("state unchanged: %s\n", state_name(ctx->state));
    } else {
        printf("state changed: %s -> %s\n",
               state_name(old_state), state_name(ctx->state));
    }
    fflush(stdout);
}

static int receive_events(bb_dev_handle_t *handle)
{
    receive_context_t receive_ctx;
    bb_set_event_callback_t callback;
    int ret;
    int unsubscribe_ret;

    memset(&receive_ctx, 0, sizeof(receive_ctx));
    receive_ctx.state = DEMO_STATE_DISARMED;

    memset(&callback, 0, sizeof(callback));
    callback.event = BB_EVENT_PRJ_DISPATCH;
    callback.callback = project_event_cb;
    callback.user = &receive_ctx;

    ret = bb_ioctl(handle, BB_SET_EVENT_SUBSCRIBE, &callback, NULL);
    if (ret) {
        printf("BB_SET_EVENT_SUBSCRIBE failed, ret=%d\n", ret);
        return ret;
    }

    printf("subscribed BB_EVENT_PRJ_DISPATCH\n");
    printf("initial state: %s; press Ctrl-C to stop\n",
           state_name(receive_ctx.state));

    while (!g_stop_requested) {
        sleep(1);
    }

    unsubscribe_ret = bb_ioctl(handle, BB_SET_EVENT_UNSUBSCRIBE, &callback, NULL);
    if (unsubscribe_ret) {
        printf("BB_SET_EVENT_UNSUBSCRIBE failed, ret=%d\n", unsubscribe_ret);
        ret = unsubscribe_ret;
    }

    return ret;
}

static int send_event(bb_dev_handle_t *handle, int slot, uint8_t state)
{
    bb_set_prj_dispatch_in_t request;
    bb_remote_ioctl_in_t remote_in;
    bb_remote_ioctl_out_t remote_out;
    prj_rpc_hdr_t *hdr;
    prj_cmd_event_demo_t *event_demo;
    int ret;

    memset(&request, 0, sizeof(request));
    hdr = (prj_rpc_hdr_t *)request.data;
    hdr->cmdid = PRJ_CMD_EVENT_DEMO;
    event_demo = (prj_cmd_event_demo_t *)hdr->data;
    event_demo->data = state;

    memset(&remote_in, 0, sizeof(remote_in));
    memset(&remote_out, 0, sizeof(remote_out));
    remote_in.slot = (uint8_t)slot;
    remote_in.len = sizeof(request);
    remote_in.msg_id = BB_SET_PRJ_DISPATCH;
    memcpy(remote_in.data, &request, sizeof(request));

    ret = bb_ioctl(handle, BB_REMOTE_IOCTL_REQ, &remote_in, &remote_out);
    if (ret) {
        printf("BB_REMOTE_IOCTL_REQ failed, slot=%d state=%s ret=%d\n",
               slot, state_name(state), ret);
        return ret;
    }

    printf("peer 8030 accepted and published event: slot=%d state=%s data=0x%02x\n",
           slot, state_name(state), state);
    return 0;
}

int main(int argc, char **argv)
{
    const char *addr = "127.0.0.1";
    int port = BB_PORT_DEFAULT;
    int dev_index = 0;
    int slot = 0;
    int opt;
    int ret;
    run_mode_t mode = RUN_MODE_NONE;
    uint8_t send_state = DEMO_STATE_DISARMED;
    bb_demo_context_t ctx;

    while ((opt = getopt(argc, argv, "ha:p:i:s:rt:")) != -1) {
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
            if (parse_int_range(optarg, 0, INT_MAX, "device index", &dev_index)) {
                return -1;
            }
            break;
        case 's':
            if (parse_int_range(optarg, 0, BB_SLOT_MAX - 1, "slot", &slot)) {
                return -1;
            }
            break;
        case 'r':
            if (mode != RUN_MODE_NONE) {
                printf("receive and send modes are mutually exclusive\n");
                return -1;
            }
            mode = RUN_MODE_RECEIVE;
            break;
        case 't':
            if (mode != RUN_MODE_NONE) {
                printf("receive and send modes are mutually exclusive\n");
                return -1;
            }
            if (parse_state(optarg, &send_state)) {
                return -1;
            }
            mode = RUN_MODE_SEND;
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    if (mode == RUN_MODE_NONE || optind != argc) {
        usage(argv[0]);
        return -1;
    }

    signal(SIGINT, stop_signal_handler);
    signal(SIGTERM, stop_signal_handler);

    ret = bb_demo_open(&ctx, addr, port, dev_index);
    if (ret) {
        return -1;
    }

    if (mode == RUN_MODE_RECEIVE) {
        ret = receive_events(ctx.handle);
    } else {
        ret = send_event(ctx.handle, slot, send_state);
    }

    bb_demo_close(&ctx);
    return ret ? -1 : 0;
}

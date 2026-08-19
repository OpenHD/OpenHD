#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "bb_api.h"
#include "debug_rpc.h"

typedef struct {
    char              addr[128];
    int               port;
    int               dev_index;
    bool              dev_index_set;
    bool              mac_set;
    char              mac[128];
    std::atomic<bool> working;
} cmd_dbg_info_t;

static void copy_string(char* dst, size_t dst_size, const char* src)
{
    if (!dst || dst_size == 0 || !src) {
        return;
    }

    std::strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static void debug_receive(struct dbg_hdl* hdl, void* priv, unsigned char* buffer, int len)
{
    (void)hdl;
    (void)priv;

    if (buffer && len > 0) {
        std::fwrite(buffer, 1, static_cast<size_t>(len), stdout);
        std::fflush(stdout);
    }
}

static void device_offline(void* arg, void* user)
{
    cmd_dbg_info_t* info = static_cast<cmd_dbg_info_t*>(user);
    (void)arg;

    std::fprintf(stderr, "device offline\n");
    info->working.store(false);
}

static int set_event_callback(bb_dev_handle_t* pdev,
                              bb_event_e event,
                              bb_event_callback callback,
                              void* user)
{
    bb_set_event_callback_t cb = {};
    cb.callback = callback;
    cb.event = event;
    cb.user = user;

    int ret = bb_ioctl(pdev, BB_SET_EVENT_SUBSCRIBE, &cb, NULL);
    if (ret != 0) {
        std::fprintf(stderr, "set event %d callback failed, ret=%d\n", event, ret);
    }

    return ret;
}

static void usage(const char* name)
{
    std::printf("Usage: %s [options]\n", name);
    std::printf("\n");
    std::printf("Options:\n");
    std::printf("  -h, --help             show this help\n");
    std::printf("  -a, --addr <addr>      daemon address, default: 127.0.0.1\n");
    std::printf("  -p, --port <port>      daemon port, default: %d\n", BB_PORT_DEFAULT);
    std::printf("  -i, --index <index>    device index, default: 0\n");
    std::printf("  -m, --mac <mac>        select device by MAC address\n");
    std::printf("  -l, --list             list devices and exit\n");
    std::printf("  -o, --output           output only, do not read commands from stdin\n");
}

int main(int argc, char* argv[])
{
    cmd_dbg_info_t info = {};
    bb_host_t* host = NULL;
    bb_dev_list_t* devs = NULL;
    bb_dev_handle_t* pdev = NULL;
    struct dbg_hdl* debug = NULL;
    bool list_only = false;
    bool output_only = false;
    int dev_count = 0;
    int result = EXIT_FAILURE;

    copy_string(info.addr, sizeof(info.addr), "127.0.0.1");
    info.port = BB_PORT_DEFAULT;
    info.dev_index = 0;
    info.working.store(true);

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"addr", required_argument, NULL, 'a'},
        {"port", required_argument, NULL, 'p'},
        {"index", required_argument, NULL, 'i'},
        {"mac", required_argument, NULL, 'm'},
        {"list", no_argument, NULL, 'l'},
        {"output", no_argument, NULL, 'o'},
        {NULL, 0, NULL, 0},
    };

    while (true) {
        int option_index = 0;
        int opt = getopt_long(argc, argv, "ha:p:i:m:lo", long_options, &option_index);
        if (opt == -1) {
            break;
        }

        switch (opt) {
        case 'h':
            usage(argv[0]);
            return EXIT_SUCCESS;
        case 'a':
            copy_string(info.addr, sizeof(info.addr), optarg);
            break;
        case 'p':
            info.port = static_cast<int>(std::strtoul(optarg, NULL, 10));
            break;
        case 'i':
            info.dev_index = static_cast<int>(std::strtoul(optarg, NULL, 10));
            info.dev_index_set = true;
            break;
        case 'm':
            copy_string(info.mac, sizeof(info.mac), optarg);
            info.mac_set = true;
            break;
        case 'l':
            list_only = true;
            break;
        case 'o':
            output_only = true;
            break;
        default:
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (optind < argc) {
        std::fprintf(stderr, "unexpected argument: %s\n", argv[optind]);
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (info.mac_set && info.dev_index_set) {
        std::fprintf(stderr, "-i/--index and -m/--mac cannot be used together\n");
        return EXIT_FAILURE;
    }

    int ret = bb_host_connect(&host, info.addr, info.port);
    if (ret != 0) {
        std::fprintf(stderr, "connect daemon failed, ret=%d\n", ret);
        goto cleanup;
    }

    dev_count = bb_dev_getlist(host, &devs);
    if (dev_count <= 0) {
        std::fprintf(stderr, "no device found, ret=%d\n", dev_count);
        goto cleanup;
    }

    if (list_only) {
        for (int i = 0; i < dev_count; ++i) {
            bb_dev_info_t dev_info = {};
            ret = bb_dev_getinfo(devs[i], &dev_info);
            if (ret == 0) {
                std::printf("device %d: mac=%s\n", i, reinterpret_cast<char*>(dev_info.mac));
            } else {
                std::printf("device %d: get info failed, ret=%d\n", i, ret);
            }
        }
        result = EXIT_SUCCESS;
        goto cleanup;
    }

    if (info.mac_set) {
        for (int i = 0; i < dev_count; ++i) {
            bb_dev_info_t dev_info = {};
            if (bb_dev_getinfo(devs[i], &dev_info) == 0 &&
                std::strcmp(reinterpret_cast<char*>(dev_info.mac), info.mac) == 0) {
                pdev = bb_dev_open(devs[i]);
                break;
            }
        }
        if (!pdev) {
            std::fprintf(stderr, "device with MAC %s not found or could not be opened\n", info.mac);
            goto cleanup;
        }
    } else {
        if (info.dev_index < 0 || info.dev_index >= dev_count) {
            std::fprintf(stderr,
                         "invalid device index %d, valid range: 0-%d\n",
                         info.dev_index,
                         dev_count - 1);
            goto cleanup;
        }

        pdev = bb_dev_open(devs[info.dev_index]);
        if (!pdev) {
            std::fprintf(stderr, "open device %d failed\n", info.dev_index);
            goto cleanup;
        }
    }

    ret = set_event_callback(pdev, BB_EVENT_OFFLINE, device_offline, &info);
    if (ret != 0) {
        goto cleanup;
    }

    debug = dbg_setup(pdev, debug_receive, &info, -1);
    if (!debug) {
        std::fprintf(stderr, "create debug session failed\n");
        goto cleanup;
    }

    while (info.working.load()) {
        if (output_only) {
            sleep(1);
            continue;
        }

        unsigned char buffer[1024];
        char* input = std::fgets(reinterpret_cast<char*>(buffer), sizeof(buffer), stdin);
        if (!input) {
            break;
        }

        if (!info.working.load()) {
            break;
        }

        dbg_write(debug, buffer, static_cast<int>(std::strlen(input)));
    }

    result = EXIT_SUCCESS;

cleanup:
    if (pdev) {
        bb_dev_close(pdev);
    }
    if (devs) {
        bb_dev_freelist(devs);
    }
    if (host) {
        bb_host_disconnect(host);
    }

    return result;
}

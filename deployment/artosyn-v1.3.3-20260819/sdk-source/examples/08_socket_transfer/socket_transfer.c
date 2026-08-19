#include "bb_demo_common.h"

#include "bb_config.h"
#include "getopt.h"
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_TRANSFER_TIMEOUT_MS 1000
#define SOCKET_TRANSFER_READ_BUF_SIZE 4096
#define SOCKET_TRANSFER_INPUT_SIZE 4096

typedef enum {
    ACTION_NONE = 0,
    ACTION_SEND_TEXT_ONCE,
    ACTION_SEND_TEXT_INPUT,
    ACTION_SEND_HEX_ONCE,
    ACTION_SEND_HEX_INPUT,
    ACTION_RECV,
} action_t;

typedef struct {
    const char *addr;
    int daemon_port;
    int dev_index;
    int slot;
    int socket_port;
    action_t action;
    const char *payload_text;
    int encrypt_enabled;
    bb_sock_encrypt_mode_t encrypt_mode;
    const char *encrypt_key_text;
    uint8_t encrypt_key[32];
} options_t;

static volatile sig_atomic_t g_stop = 0;

static void signal_handler(int sig)
{
    (void)sig;
    g_stop = 1;
}

static void setup_signal_handlers(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static void usage(const char *prog)
{
    printf("Usage: %s [options] <one action>\n", prog);
    printf("\n");
    printf("Common options:\n");
    printf("  -h, --help                 show this help\n");
    printf("  -a, --addr <addr>          daemon address, default: 127.0.0.1\n");
    printf("  -p, --port <port>          daemon port, default: %d\n", BB_PORT_DEFAULT);
    printf("  -i, --index <index>        device index, default: 0\n");
    printf("  -s, --slot <slot>          target slot, default: 0; DEV uses slot 0 as AP\n");
    printf("  -P, --socket-port <port>   socket logical port, default: 1\n");
    printf("      --encrypt-mode <mode>  enable encryption: default, aes128 or aes256\n");
    printf("      --encrypt-key <hex>    AES key: 32 hex digits for AES128, 64 for AES256\n");
    printf("\n");
    printf("Actions:\n");
    printf("  -t, --text <text>          send text bytes once, wait 1s, then close\n");
    printf("  -T, --text-input           keep reading text lines from stdin and send\n");
    printf("  -x, --hex <hex>            send hex bytes once, wait 1s, then close\n");
    printf("  -X, --hex-input            keep reading hex lines from stdin and send\n");
    printf("  -r, --recv                 receive until Ctrl-C\n");
    printf("\n");
    printf("Note:\n");
    printf("  If the shell shows dquote>, the quote is not closed and this program has not started.\n");
}

static void init_options(options_t *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->addr = "127.0.0.1";
    opts->daemon_port = BB_PORT_DEFAULT;
    opts->dev_index = 0;
    opts->slot = BB_SLOT_0;
    opts->socket_port = 1;
    opts->action = ACTION_NONE;
    opts->encrypt_mode = BB_SOCK_ENCRYPT_MODE_DEFAULT;
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

static int set_action(options_t *opts, action_t action, const char *payload)
{
    if (opts->action != ACTION_NONE) {
        printf("only one action can be specified\n");
        return -1;
    }

    opts->action = action;
    opts->payload_text = payload;
    return 0;
}

static const char *short_option_name(int opt)
{
    switch (opt) {
    case 'a':
        return "-a/--addr";
    case 'p':
        return "-p/--port";
    case 'i':
        return "-i/--index";
    case 's':
        return "-s/--slot";
    case 'P':
        return "-P/--socket-port";
    case 't':
        return "-t/--text";
    case 'x':
        return "-x/--hex";
    default:
        return "option";
    }
}

static void print_missing_argument(int opt)
{
    printf("missing argument for %s\n", short_option_name(opt));
    if (opt == 't') {
        printf("text format example: -t \"test\"\n");
        printf("if the shell shows dquote>, close the quote or press Ctrl-C\n");
    } else if (opt == 'x') {
        printf("hex format example: -x \"01 02 0a ff\"\n");
    }
}

static int has_line_break(const char *text)
{
    return strchr(text, '\n') != NULL || strchr(text, '\r') != NULL;
}

static void print_hex_format_example(void)
{
    printf("hex format example: -x \"01 02 0a ff\"\n");
    printf("each byte must use two hex digits; separators can be spaces, ':' or ','\n");
}

static int parse_hex_payload(const char *text, uint8_t **out_payload, size_t *out_len);

static int parse_encrypt_mode(const char *text, bb_sock_encrypt_mode_t *mode)
{
    if (strcmp(text, "default") == 0) {
        *mode = BB_SOCK_ENCRYPT_MODE_DEFAULT;
    } else if (strcmp(text, "aes128") == 0) {
        *mode = BB_SOCK_ENCRYPT_MODE_AES128;
    } else if (strcmp(text, "aes256") == 0) {
        *mode = BB_SOCK_ENCRYPT_MODE_AES256;
    } else {
        printf("invalid encryption mode: %s; use default, aes128 or aes256\n", text);
        return -1;
    }
    return 0;
}

static int validate_encrypt_options(options_t *opts)
{
    uint8_t *key = NULL;
    size_t key_len = 0;
    size_t expected_len;

    if (!opts->encrypt_enabled) {
        if (opts->encrypt_key_text) {
            printf("--encrypt-key requires --encrypt-mode\n");
            return -1;
        }
        return 0;
    }

    if (opts->encrypt_mode == BB_SOCK_ENCRYPT_MODE_DEFAULT) {
        if (opts->encrypt_key_text) {
            printf("default encryption mode does not use --encrypt-key\n");
            return -1;
        }
        return 0;
    }

    if (!opts->encrypt_key_text) {
        printf("%s requires --encrypt-key\n",
               opts->encrypt_mode == BB_SOCK_ENCRYPT_MODE_AES128 ? "aes128" : "aes256");
        return -1;
    }

    if (parse_hex_payload(opts->encrypt_key_text, &key, &key_len)) {
        printf("invalid --encrypt-key\n");
        return -1;
    }

    expected_len = opts->encrypt_mode == BB_SOCK_ENCRYPT_MODE_AES128 ? 16 : 32;
    if (key_len != expected_len) {
        printf("invalid encryption key length: got %zu bytes, expected %zu\n", key_len, expected_len);
        free(key);
        return -1;
    }

    memcpy(opts->encrypt_key, key, key_len);
    memset(key, 0, key_len);
    free(key);
    return 0;
}

static int parse_options(int argc, char **argv, options_t *opts)
{
    enum {
        OPT_ADDR = 1000,
        OPT_PORT,
        OPT_INDEX,
        OPT_SLOT,
        OPT_SOCKET_PORT,
        OPT_TEXT,
        OPT_TEXT_INPUT,
        OPT_HEX,
        OPT_HEX_INPUT,
        OPT_RECV,
        OPT_ENCRYPT_MODE,
        OPT_ENCRYPT_KEY,
        OPT_HELP,
    };

    static const struct option long_options[] = {
        {"addr", required_argument, NULL, OPT_ADDR},
        {"port", required_argument, NULL, OPT_PORT},
        {"index", required_argument, NULL, OPT_INDEX},
        {"slot", required_argument, NULL, OPT_SLOT},
        {"socket-port", required_argument, NULL, OPT_SOCKET_PORT},
        {"text", required_argument, NULL, OPT_TEXT},
        {"text-input", no_argument, NULL, OPT_TEXT_INPUT},
        {"hex", required_argument, NULL, OPT_HEX},
        {"hex-input", no_argument, NULL, OPT_HEX_INPUT},
        {"recv", no_argument, NULL, OPT_RECV},
        {"encrypt-mode", required_argument, NULL, OPT_ENCRYPT_MODE},
        {"encrypt-key", required_argument, NULL, OPT_ENCRYPT_KEY},
        {"help", no_argument, NULL, OPT_HELP},
        {NULL, 0, NULL, 0},
    };

    int opt;

    init_options(opts);
    opterr = 0;

    while ((opt = getopt_long(argc, argv, ":ha:p:i:s:P:t:Tx:Xr", long_options, NULL)) != -1) {
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
            if (parse_int_range(optarg, 1, 65535, "daemon port", &opts->daemon_port)) {
                return -1;
            }
            break;
        case 'i':
        case OPT_INDEX:
            if (parse_int_range(optarg, 0, 255, "device index", &opts->dev_index)) {
                return -1;
            }
            break;
        case 's':
        case OPT_SLOT:
            if (parse_int_range(optarg, BB_SLOT_0, BB_SLOT_MAX - 1, "slot", &opts->slot)) {
                return -1;
            }
            break;
        case 'P':
        case OPT_SOCKET_PORT:
            if (parse_int_range(optarg, 0, 255, "socket port", &opts->socket_port)) {
                return -1;
            }
            break;
        case 't':
        case OPT_TEXT:
            if (set_action(opts, ACTION_SEND_TEXT_ONCE, optarg)) {
                return -1;
            }
            break;
        case 'T':
        case OPT_TEXT_INPUT:
            if (set_action(opts, ACTION_SEND_TEXT_INPUT, NULL)) {
                return -1;
            }
            break;
        case 'x':
        case OPT_HEX:
            if (set_action(opts, ACTION_SEND_HEX_ONCE, optarg)) {
                return -1;
            }
            break;
        case 'X':
        case OPT_HEX_INPUT:
            if (set_action(opts, ACTION_SEND_HEX_INPUT, NULL)) {
                return -1;
            }
            break;
        case 'r':
        case OPT_RECV:
            if (set_action(opts, ACTION_RECV, NULL)) {
                return -1;
            }
            break;
        case OPT_ENCRYPT_MODE:
            if (parse_encrypt_mode(optarg, &opts->encrypt_mode)) {
                return -1;
            }
            opts->encrypt_enabled = 1;
            break;
        case OPT_ENCRYPT_KEY:
            opts->encrypt_key_text = optarg;
            break;
        case ':':
            print_missing_argument(optopt);
            return -1;
        case '?':
            printf("unknown option: -%c\n", optopt);
            usage(argv[0]);
            return -1;
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
        printf("one action is required\n");
        usage(argv[0]);
        return -1;
    }

    if (opts->action == ACTION_SEND_TEXT_ONCE && strlen(opts->payload_text) == 0) {
        printf("text payload is empty\n");
        printf("text format example: -t \"test\"\n");
        return -1;
    }

    if (opts->action == ACTION_SEND_TEXT_ONCE && has_line_break(opts->payload_text)) {
        printf("invalid text payload: -t expects one command-line string without line breaks\n");
        printf("text format example: -t \"test\"\n");
        printf("for interactive multi-line sending, use -T/--text-input\n");
        return -1;
    }

    return validate_encrypt_options(opts);
}

static int is_hex_separator(char c)
{
    return isspace((unsigned char)c) || c == ':' || c == ',';
}

static int hex_value(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }

    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return -1;
}

static int parse_hex_payload(const char *text, uint8_t **out_payload, size_t *out_len)
{
    size_t cap = strlen(text) / 2 + 1;
    size_t len = 0;
    uint8_t *payload = malloc(cap);
    const char *p = text;

    if (!payload) {
        printf("malloc failed\n");
        return -1;
    }

    while (*p) {
        int hi;
        int lo;

        while (is_hex_separator(*p)) {
            ++p;
        }

        if (*p == '\0') {
            break;
        }

        if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
            p += 2;
        }

        hi = hex_value(*p);
        if (hi < 0) {
            printf("invalid hex character: %c\n", *p);
            free(payload);
            return -1;
        }
        ++p;

        lo = hex_value(*p);
        if (lo < 0) {
            printf("hex payload must use two digits per byte\n");
            free(payload);
            return -1;
        }
        ++p;

        payload[len++] = (uint8_t)((hi << 4) | lo);
    }

    if (len == 0) {
        printf("hex payload is empty\n");
        free(payload);
        return -1;
    }

    *out_payload = payload;
    *out_len = len;
    return 0;
}

static void dump_bytes(const uint8_t *data, size_t len)
{
    size_t offset = 0;

    while (offset < len) {
        size_t line_len = len - offset;
        if (line_len > 16) {
            line_len = 16;
        }

        printf("  %04zx:", offset);
        for (size_t i = 0; i < 16; ++i) {
            if (i < line_len) {
                printf(" %02x", data[offset + i]);
            } else {
                printf("   ");
            }
        }

        printf("  |");
        for (size_t i = 0; i < line_len; ++i) {
            unsigned char c = data[offset + i];
            putchar(isprint(c) ? c : '.');
        }
        printf("|\n");

        offset += line_len;
    }
}

static void trim_line_end(char *line)
{
    size_t len = strlen(line);

    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }
}

static int write_payload(int sockfd, uint8_t *payload, size_t payload_len)
{
    int written;

    if (payload_len > UINT32_MAX) {
        printf("payload is too large: %zu\n", payload_len);
        return -1;
    }

    printf("tx %zu bytes:\n", payload_len);
    dump_bytes(payload, payload_len);

    written = bb_socket_write(sockfd, payload, (uint32_t)payload_len, SOCKET_TRANSFER_TIMEOUT_MS);
    printf("bb_socket_write requested=%zu written=%d\n", payload_len, written);

    return written == (int)payload_len ? 0 : -1;
}

static int send_interactive_line(int sockfd, action_t action, char *line)
{
    uint8_t *hex_payload = NULL;
    size_t payload_len = 0;
    int ret;

    trim_line_end(line);
    if (line[0] == '\0') {
        printf("empty line, skip\n");
        return 0;
    }

    if (action == ACTION_SEND_TEXT_INPUT) {
        return write_payload(sockfd, (uint8_t *)line, strlen(line));
    }

    ret = parse_hex_payload(line, &hex_payload, &payload_len);
    if (ret) {
        printf("invalid hex line, skip\n");
        print_hex_format_example();
        return 0;
    }

    ret = write_payload(sockfd, hex_payload, payload_len);
    free(hex_payload);
    return ret;
}

static int open_transfer_socket(bb_dev_handle_t *handle,
                                const options_t *opts,
                                uint32_t flags,
                                const char *direction)
{
    bb_sock_opt_t sock_opt = {0};
    bb_sock_opt_t *sock_opt_ptr = NULL;
    int sockfd;

    if (opts->encrypt_enabled) {
        sock_opt.tx_buf_size = BB_CONFIG_MAC_TX_BUF_SIZE;
        sock_opt.rx_buf_size = BB_CONFIG_MAC_RX_BUF_SIZE;
        sock_opt.encrypt_mode = (uint8_t)opts->encrypt_mode;
        memcpy(sock_opt.key, opts->encrypt_key, sizeof(sock_opt.key));
        sock_opt_ptr = &sock_opt;
        flags |= BB_SOCK_FLAG_ENCRYPT;
    }

    sockfd = bb_socket_open(handle,
                            (bb_slot_e)opts->slot,
                            (uint32_t)opts->socket_port,
                            flags,
                            sock_opt_ptr);
    memset(sock_opt.key, 0, sizeof(sock_opt.key));
    if (sockfd < 0) {
        printf("bb_socket_open %s failed, ret=%d\n", direction, sockfd);
        return -1;
    }

    printf("socket opened: fd=%d slot=%d port=%d mode=%s encryption=%s\n",
           sockfd,
           opts->slot,
           opts->socket_port,
           direction,
           opts->encrypt_enabled ? "enabled" : "disabled");
    return sockfd;
}

static int open_tx_socket(bb_dev_handle_t *handle, const options_t *opts)
{
    return open_transfer_socket(handle, opts, BB_SOCK_FLAG_TX, "TX");
}

static int send_once(bb_dev_handle_t *handle, const options_t *opts, uint8_t *payload, size_t payload_len)
{
    int sockfd;
    int ret = 0;

    sockfd = open_tx_socket(handle, opts);
    if (sockfd < 0) {
        return -1;
    }

    if (write_payload(sockfd, payload, payload_len)) {
        ret = -1;
        goto out;
    }

    setup_signal_handlers();
    printf("send done, wait 1s before closing socket\n");
    sleep(1);

out:
    bb_socket_close(sockfd);
    return ret;
}

static int send_input_loop(bb_dev_handle_t *handle, const options_t *opts)
{
    char line[SOCKET_TRANSFER_INPUT_SIZE];
    int sockfd;
    int ret = 0;

    sockfd = open_tx_socket(handle, opts);
    if (sockfd < 0) {
        return -1;
    }

    setup_signal_handlers();

    printf("input mode started. Enter %s payloads, Ctrl-C or EOF to close\n",
           opts->action == ACTION_SEND_TEXT_INPUT ? "text" : "hex");
    while (!g_stop) {
        printf("> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        if (send_interactive_line(sockfd, opts->action, line)) {
            ret = -1;
            break;
        }
    }

    printf("closing TX socket\n");

    bb_socket_close(sockfd);
    return ret;
}

static int receive_loop(bb_dev_handle_t *handle, const options_t *opts)
{
    uint8_t buffer[SOCKET_TRANSFER_READ_BUF_SIZE];
    uint64_t total = 0;
    int sockfd;

    sockfd = open_transfer_socket(handle, opts, BB_SOCK_FLAG_RX, "RX");
    if (sockfd < 0) {
        return -1;
    }

    setup_signal_handlers();

    printf("receiving, press Ctrl-C to stop\n");

    while (!g_stop) {
        int len = bb_socket_read(sockfd, buffer, sizeof(buffer), SOCKET_TRANSFER_TIMEOUT_MS);
        if (len <= 0) {
            continue;
        }

        total += (uint32_t)len;
        printf("rx %d bytes, total=%llu:\n", len, (unsigned long long)total);
        dump_bytes(buffer, (size_t)len);
    }

    printf("receive stopped, total=%llu bytes\n", (unsigned long long)total);
    bb_socket_close(sockfd);
    return 0;
}

int main(int argc, char **argv)
{
    options_t opts;
    bb_demo_context_t ctx;
    uint8_t *hex_payload = NULL;
    uint8_t *payload = NULL;
    size_t payload_len = 0;
    int ret;

    ret = parse_options(argc, argv, &opts);
    if (ret > 0) {
        return 0;
    }
    if (ret < 0) {
        return 1;
    }

    if (opts.action == ACTION_SEND_TEXT_ONCE) {
        payload = (uint8_t *)opts.payload_text;
        payload_len = strlen(opts.payload_text);
    } else if (opts.action == ACTION_SEND_HEX_ONCE) {
        if (parse_hex_payload(opts.payload_text, &hex_payload, &payload_len)) {
            printf("invalid hex payload for -x/--hex\n");
            print_hex_format_example();
            return 1;
        }
        payload = hex_payload;
    }

    ret = bb_demo_open(&ctx, opts.addr, opts.daemon_port, opts.dev_index);
    if (ret) {
        free(hex_payload);
        return 1;
    }

    if (opts.action == ACTION_RECV) {
        ret = receive_loop(ctx.handle, &opts);
    } else if (opts.action == ACTION_SEND_TEXT_INPUT || opts.action == ACTION_SEND_HEX_INPUT) {
        ret = send_input_loop(ctx.handle, &opts);
    } else {
        ret = send_once(ctx.handle, &opts, payload, payload_len);
    }

    bb_demo_close(&ctx);
    free(hex_payload);
    memset(opts.encrypt_key, 0, sizeof(opts.encrypt_key));
    return ret ? 1 : 0;
}

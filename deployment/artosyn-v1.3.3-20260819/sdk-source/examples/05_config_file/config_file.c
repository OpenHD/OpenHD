#include "bb_demo_common.h"

#include "getopt.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *prog)
{
    printf("Usage: %s [options] <-g file|-s file|-r>\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  -h              show this help\n");
    printf("  -a <addr>       daemon address, default: 127.0.0.1\n");
    printf("  -p <port>       daemon port, default: %d\n", BB_PORT_DEFAULT);
    printf("  -i <index>      device index, default: 0\n");
    printf("  -g <file>       dump config file with BB_GET_CFG\n");
    printf("  -s <file>       write config file with BB_SET_CFG\n");
    printf("  -r              reset config file with BB_RESET_CFG\n");
    printf("  -R              operate peer device by remote ioctl, default remote slot: 0\n");
    printf("  -S <slot>       remote slot id for -R, default: 0\n");
    printf("  -m <mode>       BB_GET_CFG mode: auto/memory/flash or 0/1/2, default: auto\n");
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

static int parse_cfg_mode(const char *text, int *out)
{
    if (str_eq_ignore_case(text, "auto")) {
        *out = 0;
        return 0;
    }

    if (str_eq_ignore_case(text, "memory")) {
        *out = 1;
        return 0;
    }

    if (str_eq_ignore_case(text, "flash")) {
        *out = 2;
        return 0;
    }

    return parse_int_range(text, 0, 2, "config mode", out);
}

static const char *cfg_mode_name(int mode)
{
    switch (mode) {
    case 0:
        return "auto";
    case 1:
        return "memory";
    case 2:
        return "flash";
    default:
        return "unknown";
    }
}

static uint16_t crc16_ccitt_update(uint16_t crc, const uint8_t *data, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000) {
                crc = (uint16_t)((crc << 1) ^ 0x1021);
            } else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }

    return crc;
}

static int config_file_ioctl(bb_dev_handle_t *handle,
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

static void print_config_title(const char *title, int remote_slot)
{
    if (remote_slot >= 0) {
        printf("\n[%s remote slot=%d]\n", title, remote_slot);
    } else {
        printf("\n[%s]\n", title);
    }
}

static int read_file(const char *path, uint8_t **data, size_t *length)
{
    FILE *fp;
    long file_size;
    uint8_t *buffer;
    size_t read_size;

    *data = NULL;
    *length = 0;

    fp = fopen(path, "rb");
    if (!fp) {
        perror("open config file failed");
        return -1;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("seek config file failed");
        fclose(fp);
        return -1;
    }

    file_size = ftell(fp);
    if (file_size <= 0) {
        printf("config file size error: %ld\n", file_size);
        fclose(fp);
        return -1;
    }

    if (file_size > UINT16_MAX) {
        printf("config file too large: %ld, max: %u\n", file_size, UINT16_MAX);
        fclose(fp);
        return -1;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        perror("seek config file failed");
        fclose(fp);
        return -1;
    }

    buffer = (uint8_t *)malloc((size_t)file_size);
    if (!buffer) {
        printf("malloc failed, size=%ld\n", file_size);
        fclose(fp);
        return -1;
    }

    read_size = fread(buffer, 1, (size_t)file_size, fp);
    fclose(fp);

    if (read_size != (size_t)file_size) {
        printf("read config file failed, expect %ld, got %zu\n", file_size, read_size);
        free(buffer);
        return -1;
    }

    *data = buffer;
    *length = (size_t)file_size;
    return 0;
}

static int dump_config_file(bb_dev_handle_t *handle, const char *path, int mode, int remote_slot)
{
    FILE *fp;
    uint16_t seq = 0;
    uint16_t offset = 0;
    size_t written = 0;
    uint16_t total_length = 0;
    uint16_t total_crc16 = 0;
    uint16_t crc = 0;
    int ret = 0;

    fp = fopen(path, "wb");
    if (!fp) {
        perror("create config file failed");
        return -1;
    }

    print_config_title("BB_GET_CFG", remote_slot);
    printf("mode=%s(%d) output=%s\n", cfg_mode_name(mode), mode, path);

    while (1) {
        bb_get_cfg_in_t input;
        bb_get_cfg_out_t output;

        memset(&input, 0, sizeof(input));
        memset(&output, 0, sizeof(output));

        input.seq = seq;
        input.mode = (uint8_t)mode;
        input.offset = offset;
        input.length = (uint16_t)sizeof(output.data);

        ret = config_file_ioctl(handle,
                                BB_GET_CFG,
                                &input,
                                (uint16_t)sizeof(input),
                                &output,
                                (uint16_t)sizeof(output),
                                remote_slot);
        if (ret) {
            printf("BB_GET_CFG failed, ret=%d seq=%u offset=%u\n", ret, seq, offset);
            break;
        }

        if (output.seq != input.seq) {
            printf("BB_GET_CFG seq mismatch, request=%u response=%u\n", input.seq, output.seq);
            ret = -1;
            break;
        }

        if (output.offset != input.offset) {
            printf("BB_GET_CFG offset mismatch, request=%u response=%u\n",
                   input.offset,
                   output.offset);
            ret = -1;
            break;
        }

        if (output.length > sizeof(output.data)) {
            printf("BB_GET_CFG invalid length=%u, max=%zu\n", output.length, sizeof(output.data));
            ret = -1;
            break;
        }

        if (seq == 0) {
            total_length = output.total_length;
            total_crc16 = output.total_crc16;
            printf("total_length=%u total_crc16=0x%04x\n", total_length, total_crc16);
        } else if (output.total_length != total_length || output.total_crc16 != total_crc16) {
            printf("BB_GET_CFG total info changed during dump\n");
            ret = -1;
            break;
        }

        if (written + output.length > total_length) {
            printf("BB_GET_CFG chunk exceeds total length, written=%zu chunk=%u total=%u\n",
                   written,
                   output.length,
                   total_length);
            ret = -1;
            break;
        }

        if (output.length > 0) {
            size_t write_size = fwrite(output.data, 1, output.length, fp);
            if (write_size != output.length) {
                printf("write config file failed, expect %u, got %zu\n", output.length, write_size);
                ret = -1;
                break;
            }

            crc = crc16_ccitt_update(crc, output.data, output.length);
            written += output.length;
        }

        printf("chunk seq=%u offset=%u length=%u\n", output.seq, output.offset, output.length);

        if (written >= total_length) {
            break;
        }

        if (output.length == 0) {
            printf("BB_GET_CFG made no progress before total length was reached\n");
            ret = -1;
            break;
        }

        if (offset > UINT16_MAX - output.length) {
            printf("BB_GET_CFG offset overflow\n");
            ret = -1;
            break;
        }

        offset = (uint16_t)(offset + output.length);
        ++seq;
    }

    if (fclose(fp) != 0 && ret == 0) {
        perror("close config file failed");
        ret = -1;
    }

    if (ret == 0 && written != total_length) {
        printf("config dump length mismatch, written=%zu total=%u\n", written, total_length);
        ret = -1;
    }

    if (ret == 0 && crc != total_crc16) {
        printf("config dump crc mismatch, calc=0x%04x total=0x%04x\n", crc, total_crc16);
        ret = -1;
    }

    if (ret == 0) {
        printf("dump config ok, bytes=%zu crc16=0x%04x\n", written, crc);
    }

    return ret;
}

static int set_config_file(bb_dev_handle_t *handle, const char *path, int remote_slot)
{
    uint8_t *data = NULL;
    size_t length = 0;
    size_t offset = 0;
    uint32_t seq = 0;
    uint16_t crc;
    int ret;

    ret = read_file(path, &data, &length);
    if (ret) {
        return ret;
    }

    crc = crc16_ccitt_update(0, data, length);

    print_config_title("BB_SET_CFG", remote_slot);
    printf("input=%s total_length=%zu total_crc16=0x%04x\n", path, length, crc);

    while (offset < length) {
        bb_set_cfg_t input;
        size_t chunk = length - offset;

        if (chunk > sizeof(input.data)) {
            chunk = sizeof(input.data);
        }

        memset(&input, 0, sizeof(input));
        input.seq = seq;
        input.total_length = (uint16_t)length;
        input.total_crc16 = crc;
        input.offset = (uint16_t)offset;
        input.length = (uint16_t)chunk;
        memcpy(input.data, data + offset, chunk);

        ret = config_file_ioctl(handle,
                                BB_SET_CFG,
                                &input,
                                (uint16_t)sizeof(input),
                                NULL,
                                0,
                                remote_slot);
        if (ret) {
            printf("BB_SET_CFG failed, ret=%d seq=%u offset=%u length=%u\n",
                   ret,
                   input.seq,
                   input.offset,
                   input.length);
            free(data);
            return ret;
        }

        printf("chunk seq=%u offset=%u length=%u\n", input.seq, input.offset, input.length);
        offset += chunk;
        ++seq;
    }

    printf("set config ok, bytes=%zu crc16=0x%04x\n", length, crc);
    free(data);
    return 0;
}

static int reset_config_file(bb_dev_handle_t *handle, int remote_slot)
{
    int ret;

    print_config_title("BB_RESET_CFG", remote_slot);
    ret = config_file_ioctl(handle, BB_RESET_CFG, NULL, 0, NULL, 0, remote_slot);
    if (ret) {
        printf("BB_RESET_CFG failed, ret=%d\n", ret);
        return ret;
    }

    printf("reset config ok\n");
    return 0;
}

int main(int argc, char **argv)
{
    const char *addr = "127.0.0.1";
    const char *get_path = NULL;
    const char *set_path = NULL;
    int port = BB_PORT_DEFAULT;
    int dev_index = 0;
    int cfg_mode = 0;
    int do_reset = 0;
    int remote = 0;
    int remote_slot = 0;
    int remote_slot_set = 0;
    int action_count = 0;
    int opt;
    int ret;
    bb_demo_context_t ctx;

    while ((opt = getopt(argc, argv, "ha:p:i:g:s:rRS:m:")) != -1) {
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
        case 'g':
            get_path = optarg;
            ++action_count;
            break;
        case 's':
            set_path = optarg;
            ++action_count;
            break;
        case 'r':
            do_reset = 1;
            ++action_count;
            break;
        case 'R':
            remote = 1;
            break;
        case 'S':
            if (parse_int_range(optarg, 0, 255, "remote slot", &remote_slot)) {
                return -1;
            }
            remote_slot_set = 1;
            break;
        case 'm':
            if (parse_cfg_mode(optarg, &cfg_mode)) {
                return -1;
            }
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    if (action_count != 1) {
        printf("specify exactly one action: -g file, -s file, or -r\n");
        usage(argv[0]);
        return -1;
    }

    if (remote_slot_set && !remote) {
        printf("-S is only valid with -R\n");
        usage(argv[0]);
        return -1;
    }

    if (cfg_mode != 0 && !get_path) {
        printf("-m is only valid with -g\n");
        usage(argv[0]);
        return -1;
    }

    ret = bb_demo_open(&ctx, addr, port, dev_index);
    if (ret) {
        return -1;
    }

    if (get_path) {
        ret = dump_config_file(ctx.handle, get_path, cfg_mode, remote ? remote_slot : -1);
    } else if (set_path) {
        ret = set_config_file(ctx.handle, set_path, remote ? remote_slot : -1);
    } else if (do_reset) {
        ret = reset_config_file(ctx.handle, remote ? remote_slot : -1);
    }

    bb_demo_close(&ctx);
    return ret ? -1 : 0;
}

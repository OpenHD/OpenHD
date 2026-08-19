#include "bb_demo_common.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void print_mac_field(const bb_dev_info_t *info)
{
    int printable = 1;

    if (info->maclen <= 0) {
        printf("(empty)");
        return;
    }

    for (int i = 0; i < info->maclen; ++i) {
        if (!isprint(info->mac[i])) {
            printable = 0;
            break;
        }
    }

    if (printable) {
        printf("%.*s", info->maclen, info->mac);
        return;
    }

    for (int i = 0; i < info->maclen; ++i) {
        printf("%s%02x", i == 0 ? "" : ":", info->mac[i]);
    }
}

void bb_demo_context_init(bb_demo_context_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->dev_index = -1;
}

void bb_demo_print_device_info(bb_dev_t *dev, int index)
{
    bb_dev_info_t info;
    int ret;

    memset(&info, 0, sizeof(info));
    ret = bb_dev_getinfo(dev, &info);
    if (ret) {
        printf("device[%d]: bb_dev_getinfo failed, ret=%d\n", index, ret);
        return;
    }

    printf("device[%d]: mac=", index);
    print_mac_field(&info);
    printf("\n");
}

int bb_demo_open(bb_demo_context_t *ctx, const char *addr, int port, int dev_index)
{
    int ret;

    bb_demo_context_init(ctx);

    printf("connect host %s:%d\n", addr, port);
    ret = bb_host_connect(&ctx->host, addr, port);
    if (ret) {
        printf("bb_host_connect failed, ret=%d\n", ret);
        return ret;
    }

    ctx->dev_count = bb_dev_getlist(ctx->host, &ctx->devs);
    if (ctx->dev_count <= 0) {
        printf("bb_dev_getlist found no device, ret=%d\n", ctx->dev_count);
        bb_demo_close(ctx);
        return -1;
    }

    printf("device count: %d\n", ctx->dev_count);
    for (int i = 0; i < ctx->dev_count; ++i) {
        bb_demo_print_device_info(ctx->devs[i], i);
    }

    if (dev_index < 0 || dev_index >= ctx->dev_count) {
        printf("invalid device index %d, valid range: 0-%d\n", dev_index, ctx->dev_count - 1);
        bb_demo_close(ctx);
        return -1;
    }

    ctx->handle = bb_dev_open(ctx->devs[dev_index]);
    if (!ctx->handle) {
        printf("bb_dev_open failed, index=%d\n", dev_index);
        bb_demo_close(ctx);
        return -1;
    }

    ctx->dev_index = dev_index;
    printf("opened device[%d]\n", dev_index);
    return 0;
}

void bb_demo_close(bb_demo_context_t *ctx)
{
    if (!ctx) {
        return;
    }

    if (ctx->handle) {
        bb_dev_close(ctx->handle);
        ctx->handle = NULL;
    }

    if (ctx->devs) {
        bb_dev_freelist(ctx->devs);
        ctx->devs = NULL;
    }

    if (ctx->host) {
        bb_host_disconnect(ctx->host);
        ctx->host = NULL;
    }

    ctx->dev_count = 0;
    ctx->dev_index = -1;
}

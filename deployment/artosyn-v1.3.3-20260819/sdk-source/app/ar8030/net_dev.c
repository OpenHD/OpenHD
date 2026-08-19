#include "bb_api.h"
#include "ar_net_api.h"
#include "bb_dev.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/ioctl.h>

static int cmd_port_handle(int fd, ar_netif_t *ar_netif)
{
    int ret = 0;

    ret = ioctl(fd, AR_CHARDEV_IOCTL_NET_HANDLE_NETIF, ar_netif);
    if (ret < 0)
        printf("ioctl: %s\n", strerror(errno));
    else
        printf("Send cmd port_handle success.\n");

    return ret;
}

AR8030_API int bb_net_dev_open(bb_dev_handle_t* dev)
{
    int fd = -1;
    int ret = 0;
    char fd_name[256] = {0};

    if (!dev) {
        return -1;
    }

    sprintf(fd_name, "/dev/ar_net%d", dev->sel_id);
    fd = open(fd_name, O_RDWR);
    if (fd == -1){
        printf("Open device(%s) failed! %s\n", fd_name, strerror(errno));
        return -2;
    }

    return fd;
}

AR8030_API int bb_net_dev_close(int net_dev_fd)
{
    if (net_dev_fd < 0) {
        printf("net_dev_fd %d is invalid\n", net_dev_fd);
        return -1;
    }

    close(net_dev_fd);

    return 0;
}

AR8030_API int bb_net_dev_create(bb_dev_handle_t* dev, ar_netif_t *ar_netif)
{
    int ret = 0;
    int net_dev_fd = 0;
    ar_netif->op_type = AR_NET_OP_CREATE;

    net_dev_fd = bb_net_dev_open(dev);
    if (net_dev_fd < 0) {
        return -1;
    }

    ret = cmd_port_handle(net_dev_fd, ar_netif);
    if (!ret) {
        printf("Create net dev success!\n");
    }
    else {
        printf("Create net dev failed! ret %d\n", ret);
    }

    bb_net_dev_close(net_dev_fd);

    return ret;
}

AR8030_API int bb_net_dev_destroy(bb_dev_handle_t* dev, unsigned char slot, unsigned short socket_port)
{
    int ret = 0;
    int net_dev_fd = 0;
    ar_netif_t ar_netif = {0};

    net_dev_fd = bb_net_dev_open(dev);
    if (net_dev_fd < 0) {
        return -1;
    }

    ar_netif.slot = slot;
    ar_netif.socket_port = socket_port;
    ar_netif.op_type = AR_NET_OP_DESTORY;

    ret = cmd_port_handle(net_dev_fd, &ar_netif);
    if (!ret) {
        printf("Destroy net dev success!\n");
    }
    else {
        printf("Destroy net dev failed! ret %d\n", ret);
    }

    bb_net_dev_close(net_dev_fd);

    return ret;
}

AR8030_API int bb_net_dev_buf_resize(bb_dev_handle_t* dev, ar_netif_t *ar_netif)
{
    int ret = 0;
    int net_dev_fd = 0;
    ar_netif->op_type = AR_NET_OP_BUF_RESIZE;

    net_dev_fd = bb_net_dev_open(dev);
    if (net_dev_fd < 0) {
        return -1;
    }

    ret = cmd_port_handle(net_dev_fd, ar_netif);
    if (!ret) {
        printf("Resize net dev success!\n");
    }
    else {
        printf("Resize net dev failed! ret %d\n", ret);
    }

    bb_net_dev_close(net_dev_fd);

    return ret;
}

AR8030_API int bb_net_dev_enc_update(bb_dev_handle_t* dev, ar_netif_t *ar_netif)
{
    int ret = 0;
    int net_dev_fd = 0;

    if (!ar_netif || (ar_netif->encrypt_en && ar_netif->encrypt_mode >= BB_SOCK_ENCRYPT_MODE_MAX)) {
        return -1;
    }

    ar_netif->op_type = AR_NET_OP_ENC_UPDATE;

    net_dev_fd = bb_net_dev_open(dev);
    if (net_dev_fd < 0) {
        return -1;
    }

    ret = cmd_port_handle(net_dev_fd, ar_netif);
    if (!ret) {
        printf("Update encrypt params of the net dev success!\n");
    } else {
        printf("Update encrypt params of the net dev failed! ret %d\n", ret);
    }

    bb_net_dev_close(net_dev_fd);

    return ret;
}

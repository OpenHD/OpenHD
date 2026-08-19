#include "uart_opt.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
typedef struct {
    int serialHandle;
} linux_serial_dat;

static uart_hd linux_uart_open(const char* devname, uart_par* ipt_par)
{
    int fd = open(devname, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        return NULL;
    }

    uart_par defpar;
    defpar.BaudRate = 115200;
    defpar.ByteSize = 8;
    defpar.Parity   = 0;
    defpar.StopBits = 0;
    uart_par* par   = &defpar;
    if (ipt_par) {
        par = ipt_par;
    }

    struct termios tty;
    cfsetispeed(&tty, par->BaudRate);
    cfsetospeed(&tty, par->BaudRate);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~CRTSCTS; // no flow control
    tty.c_cc[VMIN]  = 1;     // read doesn't block
    tty.c_cc[VTIME] = 5;
    tty.c_cflag |= CREAD | CLOCAL; // turn on READ & ignore ctrl lines

    tcsetattr(fd, TCSANOW, &tty);

    linux_serial_dat* lsd = malloc(sizeof(linux_serial_dat));

    lsd->serialHandle = fd;

    return lsd;
}

static int linux_uart_rd(uart_hd hdl, void* buff, size_t len)
{
    linux_serial_dat* lsd = (linux_serial_dat*)hdl;

    return read(lsd->serialHandle, buff, len);
}

static int linux_uart_wr(uart_hd hdl, void* buff, size_t len)
{
    linux_serial_dat* lsd = (linux_serial_dat*)hdl;

    return write(lsd->serialHandle, buff, len);
}

static int linux_uart_cl(uart_hd hdl)
{
    linux_serial_dat* lsd = (linux_serial_dat*)hdl;

    close(lsd->serialHandle);

    free(lsd);
    return 0;
}


uart_list_hd* linux_uart_list_alloc(void)
{
    uart_list_hd* phd = malloc(sizeof(uart_list_hd));
    memset(phd, 0, sizeof(uart_list_hd));
    phd->num          = 0;
    const int maxsz   = sizeof(phd->uart_dev_name);
    const int idx_off = 1;
    const int nul_off = 1;

    DIR*           dir = opendir("/dev");
    struct dirent* dp;

    int offset = 0;
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_type != DT_CHR) {
            continue;
        }

        int len = strlen(dp->d_name);
        if (offset + len + idx_off + nul_off >= maxsz) {
            continue;
        }

        char* find = strstr(dp->d_name, "tty");
        if (!find) {
            continue;
        }
        // printf("name = %s\n", dp->d_name);

        phd->uart_dev_name[offset]                 = len + nul_off;
        phd->uart_dev_name[offset + len + nul_off] = 0;
        memcpy(phd->uart_dev_name + offset + idx_off, dp->d_name, len);

        offset += (len + nul_off + idx_off);
        phd->num++;
    }

    closedir(dir);
#if 0
    printf("size = %d\n", phd->num);
    for (int i = 0; i < maxsz; i++) {
        printf("%02x ", phd->uart_dev_name[i]);
    }
#endif
    return phd;
}

int linux_uart_list_free(uart_list_hd* ipt_list)
{
    free(ipt_list);
    return 0;
}

static uart_opt opt = {
    .op = linux_uart_open,
    .rd = linux_uart_rd,
    .wr = linux_uart_wr,
    .cl = linux_uart_cl,

    .list_alloc = linux_uart_list_alloc,
    .list_free  = linux_uart_list_free,
};

uart_opt* get_back_end(void)
{
    return &opt;
}



#include "bb_config.h"
#include "com_log.h"
#include "dev8030.h"
#include "getopt.h"
#include "rpc_node.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define UDS_PATH "D:\\1"
#else
#define UDS_PATH "./1"
#endif

#ifdef WIN32
#include "CrashDump.h"
#include "time.h"
#include <direct.h>
#include <windows.h>

#define sleep(n)  Sleep(n * 1000)
#define usleep(n) Sleep(n / 1000)
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef DEV_8030_UART
#include "uart8030/uart_dev.h"
#endif

#define DEFAULT_UART_ID     1
#define OPT_LOG_FILE        1000

typedef enum intf_type_e {
    INTF_TYPE_USB,
    INTF_TYPE_SDIO,
    INTF_TYPE_UART,
    INTF_TYPE_DRV,
    INTF_TYPE_MAX
} intf_type_t;

/* port of rpc socket */
static int port = BB_PORT_DEFAULT;
/* Interface of the ioctl */
static int intf = INTF_TYPE_USB;

/* UART params */
#ifdef WIN32
static int uart_id = DEFAULT_UART_ID;
#else
static char uart_name[UART_NAME_LEN + 1] = {0};
#endif
static int baudrate = 115200;
static int parity = 0;
static int stop_bits = 0;
static int data_bits = 8;
static int save_log_file = 0;

void print_help(void)
{
    com_log(COM_INIT, "Usage: l4_daemon [options]\n");
    com_log(COM_INIT, "Options:\n");
    com_log(COM_INIT, "\t-h       Print help info \n");
    com_log(COM_INIT, "\t-p       Port of the rpc socket\n");
    com_log(COM_INIT, "\t-i       Type of the interface(0:usb,1:sdio,2:uart,3:drv)\n");
    com_log(COM_INIT, "\t-U       Uart id\n");
    com_log(COM_INIT, "\t-B       Baudrate(9600,19200,57600,115200,128000,256000)\n");
    com_log(COM_INIT, "\t-P       Parity(0:no,1:odd,2:even,3:mark,4:space)\n");
    com_log(COM_INIT, "\t-S       Stopbit(0:1 bit,1:1.5 bits,2:2 bits)\n");
    com_log(COM_INIT, "\t-D       Data bits\n");
    com_log(COM_INIT, "\t-l       Log level(0:debug,1:info,2:warn,3:error)\n");
    com_log(COM_INIT, "\t--log-file Save daemon logs to daemon_log\n");
}

int reg_8030_dev(rpc_info* prpc)
{
    int cnt = 0;
    int ret = 0;
#ifdef DEV_8030_USB
    if (intf == INTF_TYPE_USB) {
        ret = reg_usbrpc_platfrom(prpc);
        if (ret == 0) {
            cnt++;
        }
    }
#endif

#ifdef DEV_8030_SDIO
    if (intf == INTF_TYPE_SDIO) {
        const char* sdio_devs[] = {
            "/dev/artosyn_sdio",
        };
        int szsdio = sizeof(sdio_devs) / sizeof(sdio_devs[0]);
        ret        = reg_sdiorpc_platform(prpc, sdio_devs, szsdio);
        if (ret == 0) {
            cnt++;
        }
    }
#endif

#ifdef DEV_8030_DRV
    if (intf == INTF_TYPE_DRV) {
        /** Support 8 devices temperaly */
        const char* drv_devs[] = {
            "/dev/ar_mdev0",
            "/dev/ar_mdev1",
            "/dev/ar_mdev2",
            "/dev/ar_mdev3",
            "/dev/ar_mdev4",
            "/dev/ar_mdev5",
            "/dev/ar_mdev6",
            "/dev/ar_mdev7"
        };
        int szdrv = sizeof(drv_devs) / sizeof(drv_devs[0]);
        ret        = reg_drvrpc_platform(prpc, drv_devs, szdrv);
        if (ret == 0) {
            cnt++;
        }
    }
#endif

#ifdef DEV_8030_UART
    if (intf == INTF_TYPE_UART) {
        char init_uart_devs[UART_NAME_LEN] = {0};
        uart_par para = {0};

        para.BaudRate = baudrate;
        para.ByteSize = data_bits;
        para.Parity = parity;
        para.StopBits = stop_bits;

#ifdef WIN32
        if (uart_id < 10)
            snprintf(init_uart_devs, UART_NAME_LEN, "COM%d", uart_id);
        else
            snprintf(init_uart_devs, UART_NAME_LEN, "\\\\.\\COM%d", uart_id);
#else
        strncpy(init_uart_devs, uart_name, UART_NAME_LEN);
#endif
        ret = reg_uartrpc_platform(prpc, init_uart_devs, &para);
        if (ret == 0) {
            cnt++;
        }
    }
#endif
    return cnt;
}

int main(int argc, char* argv[])
{
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#else
    CrashDump crashDump;
#endif

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--log-file")) {
            save_log_file = 1;
            break;
        }
    }

    char* log_file = NULL;
    char  timebuff[1024];
    if (save_log_file) {
#ifndef WIN32
        mkdir(DAEMON_LOG_PATH, 0777);
#else
        _mkdir(DAEMON_LOG_PATH);
#endif
        time_t    res = time(NULL);
        struct tm rettm;
#ifndef WIN32
        localtime_r(&res, &rettm);
#else
        localtime_s(&rettm, &res);
#endif
        sprintf(timebuff,
                "%s/%d-%d-%d %d-%d-%d.log",
                DAEMON_LOG_PATH,
                rettm.tm_year + 1900,
                rettm.tm_mon + 1,
                rettm.tm_mday,
                rettm.tm_hour,
                rettm.tm_min,
                rettm.tm_sec);
        log_file = timebuff;
    }
    com_log_init(log_file);

    int c = 0;
    while (1) {
        int                  option_index   = 0;
        static struct option long_options[] = {
            {"help",        no_argument,           0, 'h'},
            {"port",        required_argument,     0, 'p'},
            {"uart_id",     required_argument,     0, 'U'},
            {"intf",        required_argument,     0, 'i'},
            {"baudrate",    required_argument,     0, 'B'},
            {"parity",      required_argument,     0, 'P'},
            {"stop_bits",   required_argument,     0, 'S'},
            {"data_bits",   required_argument,     0, 'D'},
            {"log_level",   required_argument,     0, 'l'},
            {"log-file",    no_argument,           0, OPT_LOG_FILE},
            { 0,            0,                     0, 0  },
        };

        c = getopt_long(argc, argv, "p:hU:i:B:P:S:D:l:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'p':
            port = (int)strtoul(optarg, NULL, 10);
            break;
        case 'U':
#ifdef WIN32
            uart_id = (int)strtoul(optarg, NULL, 10);
            com_log(COM_INIT, "uart_id %d", uart_id);
#else
            strncpy(uart_name, optarg, UART_NAME_LEN);
#endif
            break;
        case 'i':
            intf = (int)strtoul(optarg, NULL, 10);
            com_log(COM_INIT, "intf %d", intf);
            break;
        case 'B':
            baudrate = (int)strtoul(optarg, NULL, 10);
            com_log(COM_INIT, "baudrate %d", baudrate);
            break;
        case 'P':
            parity = (int)strtoul(optarg, NULL, 10);
            com_log(COM_INIT, "parity %d", parity);
            break;
        case 'S':
            stop_bits = (int)strtoul(optarg, NULL, 10);
            com_log(COM_INIT, "stop_bits %d", stop_bits);
            break;
        case 'D':
            data_bits = (int)strtoul(optarg, NULL, 10);
            com_log(COM_INIT, "data_bits %d", data_bits);
            break;
        case 'l':
            com_log_set_level("print_level", (int)strtoul(optarg, NULL, 10));
            com_log(COM_INIT, "log level %d", com_log_get_level("print_level"));
            break;
        case OPT_LOG_FILE:
            save_log_file = 1;
            break;
        case 'h':
            print_help();
            exit(0);
        default:
            break;
        }
    }

    com_log(COM_NET, "daemon init , using port %d, intf %d", port, intf);

    int       portarr[] = { port };
    rpc_info* rpc       = rpc_init(portarr, sizeof(portarr) / sizeof(portarr[0]), UDS_PATH);

    if (!rpc) {
        com_log(COM_NET, "rpc init error");
        exit(-1);
    }

    int cnt = reg_8030_dev(rpc);
    com_log(COM_INIT, "reg plat cnt = %d", cnt);

    while (1) {
        int evt = dev8030_poll(rpc);

        if (!evt) {
            usleep(100 * 1000);
        }
    }

    return 0;
}

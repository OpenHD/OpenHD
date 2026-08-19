#include "getopt.h"

#include "com_log.h"
#include "tuntap.h"
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

// print usage
int usage(void)
{
    printf("Usage: l4_tuntap [options]\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help             show this help\n");
    printf("  -a, --addr <addr>      daemon address, default: 127.0.0.1\n");
    printf("  -p, --port <port>      daemon port, default: %d\n", BB_PORT_DEFAULT);
    printf("  -i, --index <index>    device index, default: 0\n");
    printf("  -P, --transport <id>   transport id, default: 3\n");
    printf("  -I, --tap-ip <ip>      TAP device IP, required\n");
    printf("  -u, --user <user>      baseband user id, default: 0\n");
    printf("  -d, --dev <name>       TAP device name, default: tap0\n");
    printf("  -v, --debug            debug mode in ethernet transfer\n");
    printf("  -k, --force-close-on-open-fail\n");
    printf("                         force close BB socket and retry once if open fails\n");
    printf("  -r, --rx-buf <bytes>   rx buffer length, default: 40000\n");
    printf("  -t, --tx-buf <bytes>   tx buffer length, default: 60000\n");

    return 0;
}

static void print_tuntap_cfg(const bb_tun_cfg& cfg)
{
    printf("l4_tuntap args: -u %d -P %d -I %s -d %s -r %u -t %u -k %d\n",
           (int)cfg.slot_id,
           cfg.port_id,
           cfg.ip,
           cfg.devname,
           (unsigned int)cfg.rx_buf_len,
           (unsigned int)cfg.tx_buf_len,
           cfg.force_close_on_open_fail);
    fflush(stdout);
}

#define BBCOM_SESSION_DATA_HEADER_SIZE      15
#define BBCOM_RX_BUF_SIZE                   (4000)
#define BBCOM_TX_FIFO_SIZE                  (4000)

#define USR_PACKET_SYNC0                    0xFF
#define USR_PACKET_SYNC1                    0xA5
#define USR_PACKET_SYNC2                    0xAA
#define USR_PACKET_SYNC3                    0x5A
#define USR_PACKET_SYNC4                    0xFF

typedef struct
{
    uint8_t     sync[5];
    uint16_t    data_len;
    uint8_t     msg_id;
    uint8_t     rsv;
    uint8_t     user_id;
    uint8_t     channel;
    uint8_t     sum_num;
    uint8_t     cur_num;
    uint16_t    check_sum;
    uint8_t     payload[BBCOM_RX_BUF_SIZE];
} __attribute__((__packed__))  STRU_PACKET_MSG;




#define USR_FRAME_LEN                       sizeof(STRU_PACKET_MSG)

static uint8_t frame_buffer[USR_FRAME_LEN] = {0};


#define PACKET_HEADER_SIZE              15

typedef struct
{
    uint8_t                         find_header;
    uint8_t                         receiving_data;
    uint8_t                         data_length_index;
    uint8_t                         header_buf_index;
    uint8_t                         header_buf[BBCOM_SESSION_DATA_HEADER_SIZE];
    uint8_t                         rx_state;
    uint16_t                        data_buf_index;
    uint16_t                        data_length;
    uint16_t                        check_sum;
} STRU_BBComRxFIFOHeader;


typedef struct
{
    STRU_BBComRxFIFOHeader          rx_fifo_header;
    uint8_t                         rx_data_buf[BBCOM_RX_BUF_SIZE + 1];
} STRU_BBComRxFIFO;

typedef enum
{
    BB_COM_RX_HEADER = 0,
    BB_COM_RX_DATALENGTH,
    BB_COM_RX_MSG_ID,
    BB_COM_RX_HEADER_CHECKSUM,
    BB_COM_RX_DATABUFFER,
    BB_COM_RX_CHECKSUM,
} ENUM_BBComRxState;

static STRU_BBComRxFIFO network_BB_ComRxFIFO = {0};

static STRU_BBComRxFIFO network_BB_ComRxFIFO_List[4] = {0};

#define BB_RECONNECT_INTERVAL_SEC  2

typedef enum
{
    BB_RECONNECT_OK = 0,
    BB_RECONNECT_DEVICE_LIST_UNAVAILABLE,
    BB_RECONNECT_TARGET_NOT_FOUND,
    BB_RECONNECT_DEVICE_OPEN_FAILED,
    BB_RECONNECT_SOCKET_OPEN_FAILED,
} ENUM_BBReconnectResult;

static const char* bb_reconnect_result_string(ENUM_BBReconnectResult result)
{
    switch (result) {
    case BB_RECONNECT_DEVICE_LIST_UNAVAILABLE:
        return "device list unavailable";
    case BB_RECONNECT_TARGET_NOT_FOUND:
        return "target device not found";
    case BB_RECONNECT_DEVICE_OPEN_FAILED:
        return "device open failed";
    case BB_RECONNECT_SOCKET_OPEN_FAILED:
        return "socket open failed";
    case BB_RECONNECT_OK:
    default:
        return "success";
    }
}

static int bb_dev_info_valid(const bb_dev_info_t& info)
{
    return info.maclen > 0 && info.maclen <= (int)sizeof(info.mac);
}

static int bb_dev_info_equal(const bb_dev_info_t& lhs, const bb_dev_info_t& rhs)
{
    return bb_dev_info_valid(lhs) &&
           bb_dev_info_valid(rhs) &&
           lhs.maclen == rhs.maclen &&
           memcmp(lhs.mac, rhs.mac, lhs.maclen) == 0;
}

static void print_target_device(const bb_dev_info_t& info)
{
    printf("target device mac=");
    for (int i = 0; i < info.maclen; ++i) {
        printf("%s%02x", i ? ":" : "", info.mac[i]);
    }
    printf("\n");
}

static int open_bb_socket(bb_tun_cfg& cfg, bb_dev_handle_t* pdev)
{
    bb_sock_opt_t opt = {0};
    opt.rx_buf_size = cfg.rx_buf_len;
    opt.tx_buf_size = cfg.tx_buf_len;

    int fd = bb_socket_open(pdev,
                            cfg.slot_id,
                            cfg.port_id,
                            BB_SOCK_FLAG_RX | BB_SOCK_FLAG_TX,
                            &opt);
    if (fd >= 0) {
        return fd;
    }

    printf("create bb socket failed\n");
    if (!cfg.force_close_on_open_fail) {
        return -1;
    }

    bb_force_close_socket_t force_close = {0};
    force_close.slot = (uint8_t)cfg.slot_id;
    force_close.port = (uint8_t)cfg.port_id;
    int ret = bb_ioctl(pdev, BB_FORCE_CLS_SOCKET, &force_close, NULL);
    printf("BB_FORCE_CLS_SOCKET slot %u port %u ret %d\n",
           (unsigned int)force_close.slot,
           (unsigned int)force_close.port,
           ret);
    if (ret) {
        return -1;
    }

    usleep(200 * 1000);
    fd = bb_socket_open(pdev,
                        cfg.slot_id,
                        cfg.port_id,
                        BB_SOCK_FLAG_RX | BB_SOCK_FLAG_TX,
                        &opt);
    if (fd < 0) {
        printf("retry create bb socket failed\n");
        return -1;
    }

    return fd;
}

static int open_initial_bb_connection(bb_tun_cfg& cfg)
{
    bb_dev_t** devs = NULL;
    int count = bb_dev_getlist(cfg.phost, &devs);
    if (count <= 0) {
        printf("dev cnt = 0\n");
        return -1;
    }

    if (cfg.dev_index < 0 || cfg.dev_index >= count) {
        printf("invalid device index %d, valid range: 0-%d\n", cfg.dev_index, count - 1);
        bb_dev_freelist(devs);
        return -1;
    }

    bb_dev_info_t info = {0};
    int ret = bb_dev_getinfo(devs[cfg.dev_index], &info);
    if (ret || !bb_dev_info_valid(info)) {
        printf("can't identify device[%d], ret=%d\n", cfg.dev_index, ret);
        bb_dev_freelist(devs);
        return -1;
    }

    bb_dev_handle_t* pdev = bb_dev_open(devs[cfg.dev_index]);
    bb_dev_freelist(devs);
    if (!pdev) {
        printf("can't open dev!!!\n");
        return -1;
    }

    int fd = open_bb_socket(cfg, pdev);
    if (fd < 0) {
        bb_dev_close(pdev);
        return -1;
    }

    cfg.target_dev_info = info;
    cfg.target_dev_info_valid = true;
    cfg.pdev = pdev;
    cfg.bb_fd.store(fd, std::memory_order_release);
    print_target_device(info);
    printf("bb socket connected, fd=%d\n", fd);
    return 0;
}

static int open_reconnected_bb_connection(bb_tun_cfg& cfg, ENUM_BBReconnectResult& result)
{
    result = BB_RECONNECT_TARGET_NOT_FOUND;
    if (!cfg.target_dev_info_valid) {
        return -1;
    }

    bb_dev_t** devs = NULL;
    int count = bb_dev_getlist(cfg.phost, &devs);
    if (count <= 0) {
        result = BB_RECONNECT_DEVICE_LIST_UNAVAILABLE;
        return -1;
    }

    bb_dev_handle_t* pdev = NULL;
    int target_found = 0;
    for (int i = 0; i < count; ++i) {
        bb_dev_info_t info = {0};
        if (bb_dev_getinfo(devs[i], &info) == 0 &&
            bb_dev_info_equal(info, cfg.target_dev_info)) {
            target_found = 1;
            pdev = bb_dev_open(devs[i]);
            break;
        }
    }
    bb_dev_freelist(devs);

    if (!pdev) {
        result = target_found ? BB_RECONNECT_DEVICE_OPEN_FAILED : BB_RECONNECT_TARGET_NOT_FOUND;
        return -1;
    }

    int fd = open_bb_socket(cfg, pdev);
    if (fd < 0) {
        result = BB_RECONNECT_SOCKET_OPEN_FAILED;
        bb_dev_close(pdev);
        return -1;
    }

    cfg.pdev = pdev;
    cfg.bb_fd.store(fd, std::memory_order_release);
    result = BB_RECONNECT_OK;
    return fd;
}

static void close_bb_connection(bb_tun_cfg& cfg, int failed_fd)
{
    std::lock_guard<std::mutex> lock(cfg.bb_socket_mutex);
    int current_fd = cfg.bb_fd.exchange(-1, std::memory_order_acq_rel);
    if (current_fd >= 0) {
        bb_socket_close(current_fd);
    } else if (failed_fd >= 0) {
        // The writer may already have marked the fd invalid. Closing twice is
        // harmless and ensures a blocked reader is woken when the session exists.
        bb_socket_close(failed_fd);
    }

    if (cfg.pdev) {
        bb_dev_close(cfg.pdev);
        cfg.pdev = NULL;
    }

    memset(&network_BB_ComRxFIFO, 0, sizeof(network_BB_ComRxFIFO));
}

static void reconnect_bb_connection(bb_tun_cfg& cfg, int failed_fd)
{
    printf("bb socket disconnected, fd=%d; starting reconnect\n", failed_fd);
    fflush(stdout);
    close_bb_connection(cfg, failed_fd);

    unsigned int attempts = 0;
    while (1) {
        ++attempts;
        ENUM_BBReconnectResult result = BB_RECONNECT_OK;
        int fd = open_reconnected_bb_connection(cfg, result);
        if (fd >= 0) {
            printf("bb socket reconnected, fd=%d, attempt=%u\n", fd, attempts);
            fflush(stdout);
            return;
        }

        printf("bb reconnect attempt %u failed: %s; retry in %d seconds\n",
               attempts,
               bb_reconnect_result_string(result),
               BB_RECONNECT_INTERVAL_SEC);
        fflush(stdout);
        sleep(BB_RECONNECT_INTERVAL_SEC);
    }
}

static uint8_t header[] = {0xFF, 0xA5, 0xAA, 0x5A, 0xFF};
#define HEADER_SYNC_SIZE                sizeof(header)



#define ARLINK_PACKET_CHECKSUM_POS      15


static void tun_2_bb_thread(bb_tun_cfg& cfg)
{
    // allocate buffer
    unsigned char* pkg_buf = (unsigned char*)malloc(cfg.buff_max + PACKET_HEADER_SIZE);

    if (!pkg_buf) {
        printf("tun2bb alloc memory error exit !!\n");
        return;
    }
 
    while (1) {
        int rdlen = cfg.dev->read(pkg_buf + PACKET_HEADER_SIZE, cfg.buff_max);

        if (rdlen <= 0) {
            continue;
        }

        if (cfg.debugflg) {
            int t = 0;
            int idx = 0;
            char buffer[300];
            memset(buffer, 0x00, 300);
            for (t = 0; t < 24; t++) {
                idx += std::sprintf(buffer + idx, "%02X ", pkg_buf[6 + t]);
            }
        
            printf("---------> tun read = %d,  %s\n", rdlen, buffer);
        }
        

        if (cfg.debugflg) {
            com_log(COM_SOCKET_DATA, "tun read = %d", rdlen);
        }


        memcpy(pkg_buf, header, sizeof(header));

        pkg_buf[5] = rdlen & 0x0FF;
        pkg_buf[6] = (rdlen >> 8) & 0x0FF;

        pkg_buf[7]  = 0x00;
        pkg_buf[8]  = 0x00;
        pkg_buf[9]  = 0x00;
        pkg_buf[10] = 0x00;
        pkg_buf[11] = 0x00;
        pkg_buf[12] = 0x00;
        

        uint16_t u16_checkSum = 0;
        /* header checksum */
        for (int i = 0; i < rdlen; i++) {
            u16_checkSum += pkg_buf[i + PACKET_HEADER_SIZE];
            /*
            u16_checkSum = u16_checkSum & 0x0FFFF;
            **/
        }
        
        /* header checksum */
        pkg_buf[13] = (u16_checkSum & 0x0FF);
        pkg_buf[14] = (u16_checkSum >> 8) & 0x0FF;
        
        {
            std::lock_guard<std::mutex> lock(cfg.bb_socket_mutex);
            int fd = cfg.bb_fd.load(std::memory_order_acquire);
            if (fd < 0) {
                // Keep consuming TAP packets while the receive thread reconnects.
                continue;
            }

            int wrlen = bb_socket_write(fd, pkg_buf, rdlen + PACKET_HEADER_SIZE, -1);
            if (cfg.debugflg) {
                com_log(COM_SOCKET_DATA, "bb write = %d", wrlen);
            }

            if (wrlen <= 0) {
                int expected_fd = fd;
                if (cfg.bb_fd.compare_exchange_strong(expected_fd,
                                                      -1,
                                                      std::memory_order_acq_rel)) {
                    // Wake bb_socket_read(); that thread owns device discovery and reconnect.
                    bb_socket_close(fd);
                }
            }
        }
    }
}


uint8_t Network_ComFindHeader(uint8_t u8_data, STRU_BBComRxFIFO *pstBBComRxFIFO)
{
    uint32_t                    check_sum = 0;
    uint8_t                     i;
    uint8_t                     ret_value = 0;
    STRU_BBComRxFIFOHeader     *rx_header = &(pstBBComRxFIFO->rx_fifo_header);

    switch (rx_header->rx_state)
    {
    case BB_COM_RX_HEADER:
        #if 0
        if (u8_data == header[0])    // Reset flag
        {
            rx_header->header_buf_index = 0;
            rx_header->header_buf[rx_header->header_buf_index++] = u8_data;
        }
        else if (rx_header->header_buf_index < sizeof(header))    // Get header
        {
            rx_header->header_buf[rx_header->header_buf_index++] = u8_data;

            if ((rx_header->header_buf_index == sizeof(header)) &&
                 (header[0] == rx_header->header_buf[0]) &&
                 (header[1] == rx_header->header_buf[1]) &&
                 (header[2] == rx_header->header_buf[2]) &&
                 (header[3] == rx_header->header_buf[3]) &&
                 (header[4] == rx_header->header_buf[4]))
            {
                rx_header->rx_state = BB_COM_RX_DATALENGTH;
                rx_header->data_length_index = 0;
            }
        }
        #else
        if (rx_header->header_buf_index >= HEADER_SYNC_SIZE) {
            printf("---------------------------> ERR: header_buf_index >= HEADER_SYNC_SIZE \n");
            rx_header->header_buf_index = 0;
        }

        rx_header->header_buf[rx_header->header_buf_index++] = u8_data;
        if (memcmp(rx_header->header_buf, header, rx_header->header_buf_index) != 0) {
            //memset(rx_header->header_buf, 0x00, HEADER_SYNC_SIZE);
            rx_header->header_buf[0] = u8_data;
            rx_header->header_buf_index = 1;
            break;
        }

        if (rx_header->header_buf_index == HEADER_SYNC_SIZE) {
            rx_header->rx_state = BB_COM_RX_DATALENGTH;
            rx_header->data_length_index = 0;
        }
        #endif
        break;

    case BB_COM_RX_DATALENGTH:
        rx_header->header_buf[rx_header->header_buf_index++] = u8_data;

        rx_header->data_length_index++;
        if (rx_header->data_length_index >= sizeof(uint16_t))
        {
            rx_header->data_length_index = 0;

            rx_header->rx_state = BB_COM_RX_MSG_ID;
        }
        break;

    case BB_COM_RX_MSG_ID:
        rx_header->header_buf[rx_header->header_buf_index++] = u8_data;
        rx_header->rx_state = BB_COM_RX_HEADER_CHECKSUM;
        break;
    case BB_COM_RX_HEADER_CHECKSUM:
        rx_header->header_buf[rx_header->header_buf_index++] = u8_data;
        if (rx_header->header_buf_index == PACKET_HEADER_SIZE) {
            check_sum = ((rx_header->header_buf[PACKET_HEADER_SIZE - 1] & 0x0FF) << 8) + (rx_header->header_buf[PACKET_HEADER_SIZE - 2] & 0x0FF);
        
            rx_header->data_length = (rx_header->header_buf[6] << 8) + rx_header->header_buf[5];
        
            if (rx_header->data_length <= BBCOM_RX_BUF_SIZE) {
                ret_value = 1;
            }

            rx_header->check_sum = (uint16_t)check_sum;
            rx_header->rx_state = BB_COM_RX_HEADER;
            rx_header->header_buf_index = 0;
        }
        break;

    default:
        rx_header->rx_state = BB_COM_RX_HEADER;
        rx_header->header_buf_index = 0;

        break;

    }

    return ret_value;
}


uint32_t BB_ComPacketDataAnalyze(bb_tun_cfg *cfg, uint8_t *u8_RxBuf, int u8_RxLen, STRU_BBComRxFIFO *pstBBComRxFIFO)
{
    uint16_t                    i = 0;
    uint16_t                    j = 0;
    uint8_t                     chData = '\0';
    uint16_t                    check_sum = 0;
    STRU_BBComRxFIFOHeader     *rx_header = &(pstBBComRxFIFO->rx_fifo_header);

    while (u8_RxLen)
    {
        chData = *(u8_RxBuf + i);

        i++;
        u8_RxLen--;

        
        if (rx_header->find_header == 0) {
            rx_header->find_header = Network_ComFindHeader(chData, pstBBComRxFIFO);
        }

        if (1 == rx_header->find_header)
        {
            if (rx_header->data_length > BBCOM_RX_BUF_SIZE)
            {
                printf("len should not exceed: %d\n", BBCOM_RX_BUF_SIZE);

                continue;
            }

            if (0 == rx_header->receiving_data)
            {
                /* begin to receive data */
                rx_header->receiving_data  = 1;

                rx_header->data_buf_index  = 0;

                //printf("find header, data_length: %d, index i =  %d ...\n", rx_header->data_length, i);
                
            }
            else
            {
                /* go on receiving data */
                pstBBComRxFIFO->rx_data_buf[rx_header->data_buf_index++] = chData;

                /* user data all received */
                if (rx_header->data_buf_index == rx_header->data_length) //2: checksum bytes
                {
                    int wrlen = cfg->dev->write(pstBBComRxFIFO->rx_data_buf, rx_header->data_buf_index);
                    if (cfg->debugflg) {
                        com_log(COM_SOCKET_DATA, "tun write = %d", wrlen);
                    }

                    //printf("receiving_data finished,  data_length: %d, i = %d  ...\n", rx_header->data_length, i);
                    rx_header->data_buf_index = 0;
                    rx_header->receiving_data = 0;
                    rx_header->find_header = 0;
                }
            }
        }
    }

    return 0;
}

static void bb_2_tun_thread(bb_tun_cfg& cfg)
{
    // allocate buffer
    uint8_t * pkg_buf = (uint8_t *)malloc(cfg.buff_max);
    if (!pkg_buf) {
        printf("bb2tun alloc memory error exit !!\n");
        return;
    }

    while (1) {
        int fd = cfg.bb_fd.load(std::memory_order_acquire);
        if (fd < 0) {
            reconnect_bb_connection(cfg, fd);
            continue;
        }

        int len = bb_socket_read(fd, pkg_buf, cfg.buff_max, -1);
        if (len <= 0) {
            reconnect_bb_connection(cfg, fd);
            continue;
        }

        if (cfg.debugflg) {
            com_log(COM_SOCKET_DATA, "bb read raw= %d", len);
        }


        if (cfg.debugflg)
            printf("----------------------------------------> start bb tun write = %d 0x%x\n", len, len);

        #if 1
        if (len > 0) {
            BB_ComPacketDataAnalyze(&cfg, pkg_buf, len, &network_BB_ComRxFIFO);
            if (cfg.debugflg) {
                printf("-----------------------------> BB_ComPacketDataAnalyze finished\n");
            }
        }

        #else
        int wrlen = cfg.dev->write(pkg_buf, len);
        if (cfg.debugflg) {
            com_log(COM_SOCKET_DATA, "tun write = %d", wrlen);
        }
        #endif
    }
}

static int tun_test(bb_tun_cfg& cfg)
{
    int ret = bb_host_connect(&cfg.phost, cfg.addr, cfg.daemon_port);
    if (ret) {
        printf("connect failed = %d\n", ret);
        return ret;
    }

    ret = open_initial_bb_connection(cfg);
    if (ret) {
        return ret;
    }

#if 0
    if (cfg.ipset_flg) {
        cfg.tun_fd = tun_alloc(cfg.devname, cfg.ip, "255.255.255.0", cfg.mtu);
        printf("dev = %s,ip = %s,tun_fd = %d\n", cfg.devname, cfg.ip, cfg.tun_fd);
    } else {
        cfg.tun_fd = tun_alloc(cfg.devname, nullptr, nullptr, cfg.mtu);
        printf("dev = %s,tun_fd = %d\n", cfg.devname, cfg.tun_fd);
    }
#else
    cfg.dev.reset(new tuntap::tap());
    cfg.dev->name(cfg.devname);
    if (cfg.ipset_flg) {
        cfg.dev->ip(cfg.ip, 24);
        cfg.dev->mtu(cfg.mtu);
        printf("dev = %s,ip = %s,mtu = %d , tun_fd = %" PRIi64 "\n",
               cfg.devname,
               cfg.ip,
               cfg.dev->mtu(),
               (uint64_t)cfg.dev->native_handle());
    }
    cfg.dev->up();
#endif

    std::thread tun_bb(tun_2_bb_thread, std::ref(cfg));
    std::thread bb_tun(bb_2_tun_thread, std::ref(cfg));

    tun_bb.join();
    bb_tun.join();

    return 0;
}

int main(int argc, char* argv[])
{
    int         opt           = 0;
    int         flag_help     = 0;
    const char* short_options = "ha:p:i:P:I:u:d:vkr:t:";

    bb_tun_cfg    cfg;
    struct option long_options[] = {
        {"help",      no_argument,       NULL, 'h'},
        { "addr",      required_argument, NULL, 'a'},
        { "port",      required_argument, NULL, 'p'},
        { "index",     required_argument, NULL, 'i'},
        { "transport", required_argument, NULL, 'P'},
        { "tap-ip",    required_argument, NULL, 'I'},
        { "user",      required_argument, NULL, 'u'},
        { "dev",   required_argument, NULL, 'd'},
        { "debug", no_argument,       NULL, 'v'},
        { "force-close-on-open-fail", no_argument, NULL, 'k'},
        { "rx-buf",    required_argument, NULL, 'r'},
        { "tx-buf",    required_argument, NULL, 't'},
        { 0,       0,                 0,    0  },
    };

    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
        case 'h':
            flag_help = 1;
            break;
        case 'a':
            strcpy(cfg.addr, optarg);
            break;
        case 'p':
            cfg.daemon_port = strtoul(optarg, NULL, 10);
            break;
        case 'i':
            cfg.dev_index = strtoul(optarg, NULL, 10);
            break;
        case 'P':
            cfg.port_id = strtoul(optarg, NULL, 10);
            break;
        case 'I':
            strcpy(cfg.ip, optarg);
            cfg.ipset_flg = 1;
            break;
        case 'u':
            cfg.slot_id = (bb_slot_e)strtoul(optarg, NULL, 10);
            break;
        case 'd':
            strcpy(cfg.devname, optarg);
            break;
        case 'v':
            printf("Set cfg.debugflg = 1 \n");
            cfg.debugflg = 1;
            break;
        case 'k':
            printf("Set cfg.force_close_on_open_fail = 1 \n");
            cfg.force_close_on_open_fail = 1;
            break;
        case 'r':
            cfg.rx_buf_len = strtoul(optarg, NULL, 10);
            break;
        case 't':
            cfg.tx_buf_len = strtoul(optarg, NULL, 10);
            break;
        default:
            printf("unknown option\n");
            break;
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    if (flag_help) {
        return usage();
    }

    if (!cfg.ipset_flg) {
        printf("Error: -I/--tap-ip is required\n");
        usage();
        return -1;
    }

    print_tuntap_cfg(cfg);

    return tun_test(cfg);
}

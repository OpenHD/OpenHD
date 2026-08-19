#include <getopt.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ar8030.h"
#include "bb_api.h"
#include "chk.h"
#include "crc.h"

#define WRITE_RETRY_TIMES 10
#define UPGRADE_WRITE_MAX_SIZE 0x300
#define GPT_FLASH_OFFSET 0x8000
#define GPT_FLASH_SIZE 0x1000

typedef struct {
    bb_dev_handle_t* pdev;
    int id;
    pthread_t pid;
    sem_t sem;
    uint32_t addr;
    unsigned int len;
    unsigned char* data;
} ar8030_upgrade_write_param_t;

typedef void* (*thread_entry_t)(void* param);

static int g_slot = -1;
static int g_err = 0;
static unsigned int g_written_size = 0;
static unsigned int g_total_len = 0;
static bb_dev_list_t* g_devs = NULL;
static int g_dev_index = 0;

static void usage(const char* name)
{
    printf("Usage: %s [options]\n", name);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help             show this help\n");
    printf("  -a, --addr <addr>      daemon address, default: 127.0.0.1\n");
    printf("  -p, --port <port>      daemon port, default: %d\n", BB_PORT_DEFAULT);
    printf("  -i, --index <index>    device index, default: 0\n");
    printf("  -f, --file <file>      upgrade image path, required\n");
    printf("  -s, --slot <slot>      remote slot index\n");
}

static int create_detach_thread(thread_entry_t entry, void* param, pthread_t* pid)
{
    pthread_t thread_id;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&thread_id, &attr, entry, param) == 0) {
        pthread_attr_destroy(&attr);
        if (pid) {
            *pid = thread_id;
        }
        return 0;
    }

    pthread_attr_destroy(&attr);
    return -1;
}

static int ar8030_ioctl(bb_dev_handle_t* pdev,
                        uint32_t request,
                        const void* in,
                        uint16_t in_len,
                        void* out,
                        uint16_t out_len)
{
    int ret;
    bb_remote_ioctl_in_t remote_in = {0};
    bb_remote_ioctl_out_t remote_out = {0};

    if (g_slot < 0) {
        return bb_ioctl(pdev, request, in, out);
    }

    if (in_len > sizeof(remote_in.data) || out_len > sizeof(remote_out.data)) {
        printf("remote ioctl data is too large, request %#x\n", request);
        return -1;
    }

    if (in && in_len > 0) {
        memcpy(remote_in.data, in, in_len);
    }
    remote_in.len = in_len;
    remote_in.slot = (uint8_t)g_slot;
    remote_in.msg_id = request;

    ret = bb_ioctl(pdev, BB_REMOTE_IOCTL_REQ, &remote_in, &remote_out);
    if (ret == 0 && out && out_len > 0) {
        memcpy(out, remote_out.data, out_len);
    }

    return ret;
}

static int upgrade_runsys_get(bb_dev_handle_t* pdev, bb_runsys_t* runsys_idx)
{
    int ret;
    bb_get_runsys_out_t runsys_out = {0};

    if (!runsys_idx) {
        return -1;
    }

    ret = ar8030_ioctl(pdev, BB_GET_RUN_SYS, NULL, 0, &runsys_out, sizeof(runsys_out));
    if (ret != 0) {
        printf("get runsys failed, ret %d\n", ret);
        return ret;
    }

    *runsys_idx = runsys_out.runsys_id;
    return 0;
}

static int ar8030_upgrade_write(bb_dev_handle_t* pdev, uint32_t addr, uint16_t len, unsigned char* data)
{
    int retry_times = 0;
    int ret;
    bb_set_hot_upgrade_write_in_t in = {0};
    bb_set_hot_upgrade_write_out_t out = {0};

    if (len > sizeof(in.data)) {
        printf("write length %u is larger than max payload %zu\n", len, sizeof(in.data));
        return -1;
    }

    in.seq = 5673;
    in.addr = addr;
    in.len = len;
    memcpy(in.data, data, len);

    do {
        ret = ar8030_ioctl(pdev, BB_SET_HOT_UPGRADE_WRITE, &in, sizeof(in), &out, sizeof(out));
        if (ret || out.ret != 0) {
            retry_times++;
            if (retry_times > WRITE_RETRY_TIMES) {
                printf("BB_SET_HOT_UPGRADE_WRITE err, ret=%d out.ret=%d\n", ret, out.ret);
                return -1;
            }
        } else {
            break;
        }
    } while (1);

    return 0;
}

static void* ar8030_upgrade_write_thread(void* param)
{
    unsigned int i;
    int ret;
    unsigned int offset;
    ar8030_upgrade_write_param_t* write_param = (ar8030_upgrade_write_param_t*)param;

    if (!write_param) {
        return NULL;
    }

    if (!write_param->pdev || !write_param->data) {
        if (write_param->pdev) {
            bb_dev_close(write_param->pdev);
        }
        sem_post(&write_param->sem);
        return NULL;
    }

    for (i = 0; i < (write_param->len / UPGRADE_WRITE_MAX_SIZE); ++i) {
        if (g_err) {
            break;
        }

        offset = i * UPGRADE_WRITE_MAX_SIZE;
        ret = ar8030_upgrade_write(write_param->pdev,
                                   write_param->addr + offset,
                                   UPGRADE_WRITE_MAX_SIZE,
                                   write_param->data + offset);
        if (ret < 0) {
            printf("ar8030_upgrade_write failed\n");
            g_err = ret;
            break;
        }

        g_written_size += UPGRADE_WRITE_MAX_SIZE;
        printf("update process %3.2f%%\r", (float)g_written_size * 100 / g_total_len);
        fflush(stdout);
    }

    if (!g_err && (write_param->len % UPGRADE_WRITE_MAX_SIZE)) {
        offset = i * UPGRADE_WRITE_MAX_SIZE;
        ret = ar8030_upgrade_write(write_param->pdev,
                                   write_param->addr + offset,
                                   write_param->len % UPGRADE_WRITE_MAX_SIZE,
                                   write_param->data + offset);
        if (ret < 0) {
            printf("ar8030_upgrade_write failed\n");
            g_err = ret;
        } else {
            g_written_size += (write_param->len % UPGRADE_WRITE_MAX_SIZE);
        }
    }

    sem_post(&write_param->sem);
    return NULL;
}

static int ar8030_upgrade_chk_crc(bb_dev_handle_t* pdev,
                                  uint32_t addr,
                                  unsigned int len,
                                  unsigned int target_crc)
{
    int ret;
    bb_set_hot_upgrade_crc32_in_t in = {0};
    bb_set_hot_upgrade_crc32_out_t out = {0};

    in.seq = 3673;
    in.addr = addr;
    in.len = len;
    in.crc32 = target_crc;

    ret = ar8030_ioctl(pdev, BB_SET_HOT_UPGRADE_CRC32, &in, sizeof(in), &out, sizeof(out));
    if (ret || out.ret != 0) {
        printf("BB_SET_HOT_UPGRADE_CRC32 err, ret=%d out.ret=%d\n", ret, out.ret);
        return -1;
    }

    return 0;
}

static int ar8030_data_transmit(uint32_t addr,
                                unsigned int len,
                                unsigned char* data,
                                unsigned int* left_len)
{
    int ret = 0;
    int i;
    unsigned int block_len;
    int thread_num;
    static ar8030_upgrade_write_param_t* write_param = NULL;

    if (!left_len) {
        printf("%s left_len is NULL\n", __func__);
        return -1;
    }

    g_err = 0;

    if ((len / 0x1000) > BB_REMOTE_CMD_WAIT_MAX) {
        thread_num = BB_REMOTE_CMD_WAIT_MAX;
        block_len = (len / 0x1000) / thread_num * 0x1000;
    } else {
        thread_num = (int)(len / 0x1000);
        block_len = 0x1000;
    }

    *left_len = len - (block_len * thread_num);

    if (thread_num <= 0) {
        return 0;
    }

    if (!write_param) {
        write_param = (ar8030_upgrade_write_param_t*)calloc(BB_REMOTE_CMD_WAIT_MAX, sizeof(*write_param));
        if (!write_param) {
            printf("%s write_param malloc failed\n", __func__);
            return -2;
        }

        for (i = 0; i < BB_REMOTE_CMD_WAIT_MAX; i++) {
            sem_init(&write_param[i].sem, 0, 0);
            write_param[i].pdev = bb_dev_open(g_devs[g_dev_index]);
            write_param[i].id = i;
            if (!write_param[i].pdev) {
                printf("bb_dev_open failed for write thread %d\n", i);
                return -1;
            }
        }
    }

    for (i = 0; i < thread_num; i++) {
        write_param[i].addr = addr + i * block_len;
        write_param[i].data = data + i * block_len;
        write_param[i].len = block_len;
        write_param[i].id = i;
        ret = create_detach_thread(ar8030_upgrade_write_thread, &write_param[i], &write_param[i].pid);
        if (ret) {
            printf("%s create thread failed, ret %d\n", __func__, ret);
            g_err = -1;
            break;
        }
    }

    while (--i >= 0) {
        sem_wait(&write_param[i].sem);
    }

    return g_err ? g_err : ret;
}

static int ar8030_upgrade_partition(bb_dev_handle_t* pdev,
                                    uint32_t addr,
                                    unsigned int len,
                                    unsigned char* data)
{
    unsigned int o_crc;
    unsigned int left_len = len;
    unsigned int cur_len = left_len;
    int ret;

    o_crc = getcrc32(data, len);
    printf("%s addr %#" PRIx32 " len %u crc %08x\n", __func__, addr, len, o_crc);

    g_written_size = 0;
    g_total_len = len;

    while (left_len > 0x1000) {
        ret = ar8030_data_transmit(addr + g_written_size, cur_len, data + g_written_size, &left_len);
        if (ret) {
            return -1;
        }
        cur_len = left_len;
    }

    while (left_len > UPGRADE_WRITE_MAX_SIZE) {
        printf("update process %3.2f%%\r", (float)g_written_size * 100 / len);
        fflush(stdout);
        ret = ar8030_upgrade_write(pdev, addr + g_written_size, UPGRADE_WRITE_MAX_SIZE, data + g_written_size);
        if (ret < 0) {
            printf("ar8030_upgrade_write failed\n");
            return -1;
        }
        g_written_size += UPGRADE_WRITE_MAX_SIZE;
        left_len -= UPGRADE_WRITE_MAX_SIZE;
    }

    if (left_len) {
        printf("update process %3.2f%%\r", (float)g_written_size * 100 / len);
        fflush(stdout);
        ret = ar8030_upgrade_write(pdev, addr + g_written_size, (uint16_t)left_len, data + g_written_size);
        if (ret < 0) {
            printf("ar8030_upgrade_write failed\n");
            return -1;
        }
    }

    printf("update process 100%%  \n");

    ret = ar8030_upgrade_chk_crc(pdev, addr, len, o_crc);
    if (ret < 0) {
        printf("ar8030_upgrade_chk_crc failed\n");
        return -1;
    }

    return 0;
}

static int ar8030_upgrade_partition_byname(bb_dev_handle_t* pdev,
                                           struct upgrade_hdr* hdr,
                                           struct part_info* partitions,
                                           struct segment_info* segments,
                                           unsigned char* data,
                                           const char* partname)
{
    int i;
    int ret;
    int segment_idx = -1;
    uint64_t partition_addr;
    uint64_t partition_len;

    for (i = 0; i < hdr->partitions; i++) {
        if (partitions[i].is_upgrade) {
            ++segment_idx;
        }

        if (strcmp((char*)partitions[i].name, partname) == 0) {
            break;
        }
    }

    if (i >= hdr->partitions) {
        printf("partition %s not found\n", partname);
        return -1;
    }

    if (!partitions[i].is_upgrade) {
        return 0;
    }

    if (segment_idx < 0 || partitions[i].flash_offset != segments[segment_idx].flash_offset) {
        printf("partition %s info is not match, partition offset 0x%" PRIx64
               " segment offset 0x%" PRIx64 " segment index %d\n",
               partitions[i].name,
               partitions[i].flash_offset,
               segments[segment_idx].flash_offset,
               segment_idx);
        return -1;
    }

    partition_addr = segments[segment_idx].flash_offset + GPT_FLASH_OFFSET;
    partition_len = segments[segment_idx].size_decompress;
    data += segments[segment_idx].img_offset;

    if (partition_addr > UINT32_MAX || partition_len > UINT32_MAX) {
        printf("partition %s addr or len is too large\n", partname);
        return -1;
    }

    ret = ar8030_upgrade_partition(pdev, (uint32_t)partition_addr, (unsigned int)partition_len, data);
    if (ret < 0) {
        printf("upgrade partition %s failed\n", partname);
    }

    return ret;
}

static int bb_get_version(bb_dev_handle_t* bb_handle, char* buf, size_t buf_size)
{
    int ret;
    bb_get_sys_info_out_t sys_info = {0};

    ret = bb_ioctl(bb_handle, BB_GET_SYS_INFO, NULL, &sys_info);
    if (ret) {
        printf("BB_GET_SYS_INFO failed, ret=%d\n", ret);
        return -1;
    }

    snprintf(buf, buf_size, "%s", sys_info.firmware_ver);
    printf("firmware_ver: %s\n", buf);
    return 0;
}

static int read_file(const char* path, unsigned char** data, long* file_len)
{
    FILE* fp;
    long len;
    unsigned char* buf;

    fp = fopen(path, "rb");
    if (!fp) {
        printf("open %s failed\n", path);
        return -1;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }

    len = ftell(fp);
    if (len <= 0) {
        fclose(fp);
        printf("invalid file size %ld\n", len);
        return -1;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }

    buf = (unsigned char*)malloc((size_t)len);
    if (!buf) {
        fclose(fp);
        printf("malloc %ld bytes failed\n", len);
        return -1;
    }

    if (fread(buf, 1, (size_t)len, fp) != (size_t)len) {
        free(buf);
        fclose(fp);
        printf("read %s failed\n", path);
        return -1;
    }

    fclose(fp);
    *data = buf;
    *file_len = len;
    return 0;
}

int main(int argc, char** argv)
{
    int ret = 0;
    int port = BB_PORT_DEFAULT;
    const char* file_path = NULL;
    const char* addr = "127.0.0.1";
    uint32_t partition_addr;
    bb_runsys_t runsys_idx = BB_RUNSYS_MASTER;
    bb_host_t* phost = NULL;
    bb_dev_handle_t* pdev = NULL;
    unsigned char* img_data = NULL;
    long file_len = 0;
    struct img8030 img;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"addr", required_argument, 0, 'a'},
        {"port", required_argument, 0, 'p'},
        {"index", required_argument, 0, 'i'},
        {"file", required_argument, 0, 'f'},
        {"slot", required_argument, 0, 's'},
        {0, 0, 0, 0},
    };

    while (1) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "ha:p:i:f:s:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            usage(argv[0]);
            return 0;
        case 'a':
            addr = optarg;
            break;
        case 'p':
            port = (int)strtoul(optarg, NULL, 10);
            break;
        case 'i':
            g_dev_index = (int)strtoul(optarg, NULL, 10);
            break;
        case 'f':
            file_path = optarg;
            break;
        case 's':
            g_slot = (int)strtoul(optarg, NULL, 10);
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    printf("%s compiled at: %s %s\n", argv[0], __DATE__, __TIME__);

    if (!file_path) {
        printf("Error: -f/--file is required\n");
        usage(argv[0]);
        ret = -1;
        goto end;
    }

    printf("upgrade file: %s\n", file_path);

    ret = read_file(file_path, &img_data, &file_len);
    if (ret) {
        goto end;
    }

    printf("file size = %ld\n", file_len);
    dump_upgrade_file(img_data);

    memset(&img, 0, sizeof(img));
    ret = do_verify_image(img_data, &img);
    if (ret < 0) {
        goto end;
    }

    ret = bb_host_connect(&phost, addr, port);
    if (ret) {
        printf("connect failed, ret=%d\n", ret);
        goto end;
    }

    ret = bb_dev_getlist(phost, &g_devs);
    if (ret <= 0) {
        printf("dev cnt = %d\n", ret);
        ret = -1;
        goto end;
    }

    if (g_dev_index < 0 || g_dev_index >= ret) {
        printf("invalid device index %d, valid range: 0-%d\n", g_dev_index, ret - 1);
        ret = -1;
        goto end;
    }

    pdev = bb_dev_open(g_devs[g_dev_index]);
    if (!pdev) {
        printf("can't open dev\n");
        ret = -1;
        goto end;
    }

    if (img.hdr->rom_size) {
        printf("update romcode data=%p, len=0x%x\n", img.romcode, img.hdr->rom_size);
        ret = ar8030_upgrade_partition(pdev, 0, img.hdr->rom_size, img.romcode);
        if (ret < 0) {
            goto end;
        }
    }

    {
        char ver[64] = {0};
        if (bb_get_version(pdev, ver, sizeof(ver)) != 0) {
            printf("get firmware version failed\n");
        }
    }

    ret = upgrade_runsys_get(pdev, &runsys_idx);
    if (ret != 0) {
        printf("can not read runsys, default to BB_RUNSYS_MASTER\n");
        runsys_idx = BB_RUNSYS_MASTER;
    } else {
        printf("running sys is %d\n", runsys_idx);
    }

    if (runsys_idx == BB_RUNSYS_MASTER) {
        partition_addr = img.hdr->rom_size + GPT_FLASH_SIZE;
        ret = ar8030_upgrade_partition(pdev, partition_addr, GPT_FLASH_SIZE, img.gpt);
        if (ret < 0) {
            goto end;
        }

        ret = ar8030_upgrade_partition_byname(pdev, img.hdr, img.partitions, img.segments, img_data, "app1");
        if (ret < 0) {
            goto end;
        }

        partition_addr = img.hdr->rom_size;
        ret = ar8030_upgrade_partition(pdev, partition_addr, GPT_FLASH_SIZE, img.gpt);
        if (ret < 0) {
            goto end;
        }

        ret = ar8030_upgrade_partition_byname(pdev, img.hdr, img.partitions, img.segments, img_data, "app0");
        if (ret < 0) {
            goto end;
        }
    } else {
        partition_addr = img.hdr->rom_size;
        ret = ar8030_upgrade_partition(pdev, partition_addr, GPT_FLASH_SIZE, img.gpt);
        if (ret < 0) {
            goto end;
        }

        ret = ar8030_upgrade_partition_byname(pdev, img.hdr, img.partitions, img.segments, img_data, "app0");
        if (ret < 0) {
            goto end;
        }

        partition_addr = img.hdr->rom_size + GPT_FLASH_SIZE;
        ret = ar8030_upgrade_partition(pdev, partition_addr, GPT_FLASH_SIZE, img.gpt);
        if (ret < 0) {
            goto end;
        }

        ret = ar8030_upgrade_partition_byname(pdev, img.hdr, img.partitions, img.segments, img_data, "app1");
        if (ret < 0) {
            goto end;
        }
    }

    printf("upgrade done\n");

end:
    if (pdev) {
        bb_dev_close(pdev);
    }
    if (g_devs) {
        bb_dev_freelist(g_devs);
    }
    if (phost) {
        bb_host_disconnect(phost);
    }
    free(img_data);

    return ret == 0 ? 0 : -1;
}

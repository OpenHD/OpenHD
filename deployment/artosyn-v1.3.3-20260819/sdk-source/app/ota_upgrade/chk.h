#ifndef __CHK_H__
#define __CHK_H__
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

struct upgrade_hdr {
    uint32_t magic;
    uint8_t  hdr_version;
    uint8_t  compressed;
    uint8_t  flashtype;   /* 0:spinor 1:emmc 2:spinand */
    uint8_t  part_status; /* whether partitions are changed */
    uint16_t header_ext_size;
    uint16_t hash_size;
    uint16_t sig_size;
    uint16_t sig_realsize;
    /* size of the image which lays after signature */
    uint64_t img_size;
    uint32_t rom_size;
    uint32_t loader_size;
    uint16_t partitions;
    uint16_t segments;
    uint32_t object_version; // object_version > depend_version
    uint32_t depend_version;
    uint8_t  reserve[20];
    uint8_t  part_flag[64];    /* 64 */
    uint8_t  sdk_version[128]; /* 128 */
}__attribute__((__packed__));

/* Upgrade image segment table */
struct segment_info {
    uint64_t img_offset;
    uint64_t flash_offset;
    uint64_t size_compress;
    uint64_t size_decompress;
}__attribute__((__packed__));

/* Upgrade parition table entry */
struct part_info {
    uint8_t  name[32];
    uint64_t flash_offset;
    uint64_t length;

    uint32_t is_upgrade;
}__attribute__((__packed__));

enum part_status {
    PART_STATUS_NOCHANGE = 0,
    PART_STATUS_CHANGE,
    PART_STATUS_UNKNOWN
};

struct img8030 {
    struct upgrade_hdr*  hdr;
    unsigned char*       hdr_ext;
    unsigned char*       romcode;
    unsigned char*       bootloader;
    unsigned char*       gpt;
    struct part_info*    partitions;
    struct segment_info* segments;
    char*                img;
};

int dump_upgrade_file(unsigned char* img);
int do_verify_image(unsigned char* img, struct img8030* outdat);

#endif


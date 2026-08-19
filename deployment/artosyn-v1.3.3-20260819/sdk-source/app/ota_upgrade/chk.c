#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include "chk.h"

#define 	OTA_IMG_MAGIC 			(0x4152544f)

#define     GPT_FLASH_OFFSET        (0x8000)
#define     GPT_FLASH_SIZE          (0x1000)

void print_buf(unsigned char* buf, int len)
{
    int i = 0;

    for (i = 0; i < len; ++i) {
        printf("%02x ", buf[i]);
        if (0 == ((i + 1) % 16) || (i == len - 1))
            printf("\n");
    }
}

int dump_upgrade_file(unsigned char* img)
{
    unsigned char*       buf = (unsigned char*)img;
    int                  i;
    struct upgrade_hdr*  hdr = (struct upgrade_hdr*)buf;
    struct part_info*    parts;
    struct segment_info* segments;

    printf("\n====================upgrade bin infomation====================\n\n");
    printf("magic:            0x%x\n", (hdr->magic));
    printf("hdr_version:      0x%x\n", hdr->hdr_version);
    printf("compressed:       0x%x\n", hdr->compressed);
    printf("flashtype:        0x%x\n", hdr->flashtype);
    printf("header_ext size:  0x%x\n", (hdr->header_ext_size));
    printf("hash size:        0x%x\n", (hdr->hash_size));
    printf("sig size:         0x%x\n", (hdr->sig_size));
    printf("sig_realsize:     0x%x\n", (hdr->sig_realsize));
    printf("img size:         0x%" PRIx64 "\n", hdr->img_size);
    printf("rom_size:         0x%x\n", (hdr->rom_size));
    printf("loader_size:      0x%x\n", (hdr->loader_size));
    printf("partitions:       0x%x\n", (hdr->partitions));
    printf("segments:         0x%x\n", (hdr->segments));
    printf("obj_version:      0x%x\n", (hdr->object_version));
    printf("dep_version:      0x%x\n", (hdr->depend_version));
    if (hdr->part_status == PART_STATUS_NOCHANGE)
        printf("part_status:      no change\n");
    else if (hdr->part_status == PART_STATUS_CHANGE)
        printf("part_status:      changed\n");
    else
        printf("part_status:      unknown\n");
    printf("\n");

    buf += sizeof(*hdr);
    buf += (hdr->header_ext_size); /* now is 0 */
    printf("Hash:\n");
    print_buf(buf, 32);
    printf("\n");

    buf += 32;
    printf("RSA Signature:\n");
    print_buf(buf, 256);
    printf("\n");

    buf += 256;
    buf += (hdr->rom_size) + (hdr->loader_size) + GPT_FLASH_SIZE;

    parts = (struct part_info*)buf;

    /* init here, not when declarations, because uboot command upgrade many times */
    // partitions_size = 0;
    printf("Upgrade image partition table:\n");
    for (i = 0; i < (hdr->partitions); i++) {
        printf("part %d, name %8s, length 0x%08" PRIx64 ", upgrade %d\n", i, parts->name, parts->length, parts->is_upgrade);
        // partitions_size += (parts->length);
        parts++;
    }

    printf("\n");

    buf += sizeof(struct part_info) * (hdr->partitions);
    segments = (struct segment_info*)buf;

    printf("Upgrade image segments:\n");
    for (i = 0; i < (hdr->segments); i++) {
        printf("segment %03d, img offset=0x%08" PRIx64 ", flash offset=0x%08" PRIx64 ", compress size=0x%08" PRIx64 ", decompress "
               "size=0x%08" PRIx64 "\n",
               i,
               segments->img_offset,
               segments->flash_offset,
               segments->size_compress,
               segments->size_decompress);
        segments++;
    }

    printf("\n====================upgrade bin infomation done====================\n\n\n");

    return 0;
}

int get_part_info_offset(struct part_info* part_info, int nrparts, struct segment_info* segment)
{
    int i;

    /* avoid segments spreading over two partitions */
    for (i = 0; i < nrparts; i++) {
        if ((segment->flash_offset) >= (part_info[i].flash_offset)
            && (segment->flash_offset) + (segment->size_decompress)
            <= (part_info[i].flash_offset) + (part_info[i].length)) {
            return i;
        }
    }

    return -1;
}

int do_verify_image(unsigned char* img, struct img8030* outdat)
{
    unsigned char *hash, *data, *sig;

    int ret = 0;

    int                i, m, n;
    int                index_tmp, index;
    unsigned long long img_size;

    printf("Verify upgrade image...\n");

    /* Verify header */
    outdat->hdr = (struct upgrade_hdr*)img;

    if ((outdat->hdr->magic) != OTA_IMG_MAGIC || (outdat->hdr->hash_size) != 32 || (outdat->hdr->sig_size) != 256) {
        printf("error %d , %x %x %x\n", __LINE__, outdat->hdr->magic, outdat->hdr->hash_size, outdat->hdr->sig_size);
        return -1;
    }

    hash = img + sizeof(*outdat->hdr);
    sig  = hash + 32;
    data = sig + 256;

    outdat->hdr        = (struct upgrade_hdr*)img;
    outdat->hdr_ext    = (unsigned char*)img + sizeof(*outdat->hdr);
    outdat->romcode    = outdat->hdr_ext + 32 + 256 + (outdat->hdr->header_ext_size);
    outdat->bootloader = outdat->romcode + (outdat->hdr->rom_size);
    outdat->gpt        = outdat->bootloader + (outdat->hdr->loader_size);
    outdat->partitions = (struct part_info*)(outdat->gpt + GPT_FLASH_SIZE);
    outdat->segments   = (struct segment_info*)(outdat->partitions + outdat->hdr->partitions);

    if ((outdat->hdr->rom_size) && (outdat->hdr->rom_size) > 0x8000) {
        printf("error %d\n", __LINE__);
        return -1;
    }

    if ((outdat->hdr->loader_size) && (outdat->hdr->loader_size) > 0x100000) {
        printf("error %d\n", __LINE__);
        return -1;
    }

    m = (outdat->hdr->partitions);
    n = (outdat->hdr->segments);

    index     = -1;
    index_tmp = -1;
    img_size  = 0;
    for (i = 0; i < n; i++) {
        index_tmp = get_part_info_offset(outdat->partitions, m, &outdat->segments[i]);
        if (index_tmp < 0) {
            printf("get part info based on segment flash offset failed\n");
            return -1;
        }

        /* the first partition */
        if (index == -1) {
            index = index_tmp;
        }

        /* the next partition */
        if (index != index_tmp) {
            if (img_size > (outdat->partitions[index].length)) {
                printf("image is too big, partition name: %s\n", outdat->partitions[index].name);
                return -1;
            }
            index    = index_tmp;
            img_size = 0;
        }

        img_size += (outdat->segments[i].size_decompress);
    }

    /* the last partition */
    if (img_size > (outdat->partitions[index].length)) {
        printf("image is too big, partition name: %s\n", outdat->partitions[index].name);
        return -1;
    }

    printf("Verify upgrade image success\n");

    return 0;
}

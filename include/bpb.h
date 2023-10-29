/* bpb.h
 *
 * BIOS Parameter Block and Extended Boot Record for FAT32
 * created on 2011 Oct 30 14:52:38
 */
#ifndef BPB_H
#define BPB_H

#include "types.h"

/* BIOS Parameter Block */
typedef struct _bpb {
     uint8_t bs_jmp_boot[3];
     uint8_t bs_oem_name[8];
    uint16_t bytes_per_sec;
     uint8_t sectors_per_cluster;
    uint16_t rsvd_sec_cnt;
     uint8_t num_fats;
    uint16_t root_ent_cnt;
    uint16_t tot_sec_16;
     uint8_t media;
    uint16_t fat_sz_16;
    uint16_t sec_per_trk;
    uint16_t num_heads;
    uint32_t hidd_sec;
    uint32_t tot_sec_32;
}__attribute__((packed)) bpb_t;

/* Extended Boot Record */
typedef struct _ebr {
    /* FAT32-specific structure at offset 36 */
    uint32_t fat_sz_32;
    uint16_t ext_flags;
    uint16_t fs_ver;
    uint32_t root_clus;
    uint16_t fs_info;
    uint16_t bk_boot_sec;
    uint8_t reserved[12];
    uint8_t drv_num;
    uint8_t reserved1;
    uint8_t boot_sig;
    uint32_t vol_id;
    uint8_t vol_lab[11];
    uint8_t fil_sys_type[8];
}__attribute__((packed)) ebr_t;

#endif

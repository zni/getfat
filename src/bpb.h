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
     u8_t bs_jmp_boot[3];
     u8_t bs_oem_name[8];
    u16_t bytes_per_sec;
     u8_t sectors_per_cluster;
    u16_t rsvd_sec_cnt;
     u8_t num_fats;
    u16_t root_ent_cnt;
    u16_t tot_sec_16;
     u8_t media;
    u16_t fat_sz_16;
    u16_t sec_per_trk;
    u16_t num_heads;
    u32_t hidd_sec;
    u32_t tot_sec_32;
}__attribute__((packed)) bpb_t;

/* Extended Boot Record */
typedef struct _ebr {
    /* FAT32-specific structure at offset 36 */
    u32_t fat_sz_32;
    u16_t ext_flags;
    u16_t fs_ver;
    u32_t root_clus;
    u16_t fs_info;
    u16_t bk_boot_sec;
     u8_t reserved[12];
     u8_t drv_num;
     u8_t reserved1;
     u8_t boot_sig;
    u32_t vol_id;
     i8_t vol_lab[11];
     i8_t fil_sys_type[8];
}__attribute__((packed)) ebr_t;

#endif

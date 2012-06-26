#ifndef FS_INFO_H
#define FS_INFO_H

#include "types.h"

typedef struct _fs_info {
     u8_t sector_sig[4];
     u8_t sector_sig_2[4];
    i32_t free_clusters;
    u32_t rec_alloc_clus;
     u8_t reserved[12];
     u8_t sector_sig_3[4];
}__attribute__((packed)) fsinfo_t;

typedef enum cluster_ {
    FREE       = 0x00000000,
    RESERVED   = 0x00000001,
    BAD_SECTOR = 0x0FFFFFF7,
    EOC        = 0x0FFFFFFF,
} cluster_t;
#endif

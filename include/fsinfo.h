#ifndef FS_INFO_H
#define FS_INFO_H

#include "types.h"

typedef struct _fs_info {
     uint8_t sector_sig[4];
     uint8_t sector_sig_2[4];
    int32_t free_clusters;
    uint32_t rec_alloc_clus;
     uint8_t reserved[12];
     uint8_t sector_sig_3[4];
}__attribute__((packed)) fsinfo_t;
#endif

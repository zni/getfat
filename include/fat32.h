#ifndef FAT32_H
#define FAT32_H

#include "bpb.h"
#include "dir.h"
#include "fsinfo.h"
#include "options.h"
#include "types.h"

void create_fs(options_t *);
void read_fs(options_t *);

typedef struct volume_ {
    u64_t fat_begin_lba;
    u64_t cluster_begin_lba;
    u64_t sectors_per_cluster;
    u64_t root_dir_first_cluster;
} vol_t;
#endif

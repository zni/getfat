#ifndef FAT32_H
#define FAT32_H

#include "bpb.h"
#include "dir.h"
#include "fsinfo.h"
#include "options.h"
#include "types.h"

/*
 * It's probably worth mentioning the different
 * cluster values for posterity.
 * (From http://www.isdaman.com/alsos/protocols/fats/nowhere/FAT.HTM)
 *
 * Available    0x00000000
 * Reserved     0x00000001
 * User Data    0x00000002-0x0FFFFFF6
 * Bad Cluster  0x0FFFFFF7
 * End Marker   0x0FFFFFF8-0x0FFFFFFF
 *
 */

typedef struct volume_ {
    FILE     *disk;
    bpb_t    *bpb;
    ebr_t    *ebr;
    fsinfo_t *fsinfo;
    fat_t    *fat;

    uint64_t    fat_begin_lba;
    uint32_t    cluster_begin_lba;
    uint64_t    sectors_per_cluster;
    uint64_t    root_dir_first_cluster;
} vol_t;

vol_t* create_fs(options_t *);
vol_t* read_fs(options_t *);
vol_t* free_vol(vol_t *);


#endif

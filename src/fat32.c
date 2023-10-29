/****************************************************************
 * FAT32 library functions
 *
 * Author : Matt Godshall
 * Date   : 2011 Nov 12 21:06:27
 *
 * This file is part of getfat.
 *
 * getfat is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * getfat is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * getfat. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log.h>
#include <fat32.h>


/****************************************************************
 * Look up the n-th cluster.
 */
uint32_t lookup_cluster(uint32_t cluster)
{
    return 0;
}


/****************************************************************
 * Allocate and initialize a BIOS Parameter Block.
 */
bpb_t* init_bpb(options_t *opt)
{
    printf("%s\n", __FUNCTION__);

    bpb_t *bpb = (bpb_t *) malloc(sizeof(bpb_t));

    if (!bpb) return NULL;

    bpb->bs_jmp_boot[0] = 0xEB;
    bpb->bs_jmp_boot[1] = 0x00;
    bpb->bs_jmp_boot[2] = 0x90;

    bpb->bs_oem_name[0] = 'G';
    bpb->bs_oem_name[1] = 'E';
    bpb->bs_oem_name[2] = 'T';
    bpb->bs_oem_name[3] = 'F';
    bpb->bs_oem_name[4] = 'A';
    bpb->bs_oem_name[5] = 'T';
    bpb->bs_oem_name[6] = ' ';
    bpb->bs_oem_name[7] = ' ';

    // Variable
    bpb->bytes_per_sec       = opt->sector_size;
    bpb->sectors_per_cluster = opt->cluster_size;

    // Mostly fixed.
    bpb->rsvd_sec_cnt = opt->reserved_sectors;
    bpb->num_fats     = 2;
    bpb->root_ent_cnt = 0;
    bpb->tot_sec_16   = 0;
    bpb->media        = 0xF8;
    bpb->fat_sz_16    = 0;
    bpb->sec_per_trk  = 0;
    bpb->num_heads    = 0;
    bpb->hidd_sec     = 0;

    // Convert device size to bytes and divide by sector size.
    bpb->tot_sec_32   = (opt->device_size * 1024 * 1024) / bpb->bytes_per_sec;
    printf("\ttotal sectors: %d\n", bpb->tot_sec_32);

    return bpb;
}


/****************************************************************
 * Allocate and initialize the Extended Boot Record.
 */
ebr_t* init_ebr(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);

    int clusters = volume_info->bpb->tot_sec_32 /
        volume_info->bpb->sectors_per_cluster;
    ebr_t *ebr = (ebr_t *) malloc(sizeof(ebr_t));

    if (!ebr) return NULL;

    // XXX Should be the number of sectors the FAT takes up.
    ebr->fat_sz_32   = (clusters * 4) / volume_info->bpb->bytes_per_sec;
    ebr->ext_flags   = 0;
    ebr->fs_ver      = 0;
    ebr->root_clus   = 2;
    ebr->fs_info     = 1;
    ebr->bk_boot_sec = 6;

    memset(ebr->reserved, 0, 12);

    ebr->drv_num     = 0x80;
    ebr->reserved1   = 0;
    ebr->boot_sig    = 0x29;
    ebr->vol_id      = 0xDEADBEEF;
    strncpy((char *) ebr->vol_lab, "FATNESS    ", 11);
    strncpy((char *) ebr->fil_sys_type, "FAT32   ", 8);

    return ebr;
}


/****************************************************************
 * Allocate and initialize the File System Info Block.
 */
fsinfo_t* init_fsinfo(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);

    fsinfo_t *fsinfo = (fsinfo_t *) malloc(sizeof(fsinfo_t));

    if (!fsinfo) return NULL;

    fsinfo->sector_sig[0] = 'R';
    fsinfo->sector_sig[1] = 'R';
    fsinfo->sector_sig[2] = 'a';
    fsinfo->sector_sig[3] = 'A';

    fsinfo->sector_sig_2[0] = 'r';
    fsinfo->sector_sig_2[1] = 'r';
    fsinfo->sector_sig_2[2] = 'A';
    fsinfo->sector_sig_2[3] = 'a';

    fsinfo->free_clusters = 0xFFFFFFFF;
    fsinfo->rec_alloc_clus = 0;

    memset(fsinfo->reserved, 0, 12);

    fsinfo->sector_sig_3[0] = 0x00;
    fsinfo->sector_sig_3[1] = 0x00;
    fsinfo->sector_sig_3[2] = 0x55;
    fsinfo->sector_sig_3[3] = 0xAA;

    return fsinfo;
}


/****************************************************************
 * Allocate and initialize the file allocation table (FAT).
 */
fat_t* init_fat(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);
    printf("\ttotal clusters: %d\n",
        (volume_info->bpb->tot_sec_32 / volume_info->bpb->sectors_per_cluster));

    fat_t *fat = (fat_t*) calloc(
        (volume_info->bpb->tot_sec_32 / volume_info->bpb->sectors_per_cluster),
        sizeof(fat_t));

    if (!fat) {
        fprintf(stderr, "\tfat not allocated.\n");
        return NULL;
    }

    /* Mark end of cluster. */
    fat[0] = 0xFFFFFFF8;

    return fat;
}


/****************************************************************
 * Allocate and initialize the volume information.
 *
 * vol_t provides information on the volume, such as:
 * the LBA of the FAT, the LBA of the first cluster,
 * and other fun things.
 *
 */
vol_t* calc_volume_info(vol_t* volume_info)
{
    printf("%s\n", __FUNCTION__);
    static const uint8_t PARTITION_BEGIN_LBA = 0;

    volume_info->fat_begin_lba = PARTITION_BEGIN_LBA +
        volume_info->bpb->rsvd_sec_cnt;
    volume_info->cluster_begin_lba = PARTITION_BEGIN_LBA +
        volume_info->bpb->rsvd_sec_cnt +
        (volume_info->bpb->num_fats * volume_info->bpb->tot_sec_32);
    volume_info->sectors_per_cluster = volume_info->bpb->sectors_per_cluster;
    volume_info->root_dir_first_cluster = volume_info->ebr->root_clus;

    return volume_info;
}


/****************************************************************
 * Write the BIOS Parameter Block to the volume.
 * Total size: 36 bytes
 */
void write_bpb(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);
    long int tell = 0;
    fseek(volume_info->disk, 0, SEEK_SET);
    tell = ftell(volume_info->disk);
    printf("\tafter seek: %ld\n", tell);

    fwrite(volume_info->bpb->bs_jmp_boot, sizeof(uint8_t), 3, volume_info->disk);
    fwrite(volume_info->bpb->bs_oem_name, sizeof(uint8_t), 8, volume_info->disk);
    fwrite(&(volume_info->bpb->bytes_per_sec), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->sectors_per_cluster), sizeof(uint8_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->rsvd_sec_cnt), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->num_fats), sizeof(uint8_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->root_ent_cnt), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->tot_sec_16), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->media), sizeof(uint8_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->fat_sz_16), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->sec_per_trk), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->num_heads), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->hidd_sec), sizeof(uint32_t), 1, volume_info->disk);
    fwrite(&(volume_info->bpb->tot_sec_32), sizeof(uint32_t), 1, volume_info->disk);
}


/****************************************************************
 * Write the Extended Boot Record to the volume.
 * Total size: 476 bytes
 */
void write_ebr(vol_t *volume_info)
{
    long int tell = 0;
    printf("%s\n", __FUNCTION__);
    fseek(volume_info->disk, 36, SEEK_SET);
    tell = ftell(volume_info->disk);
    printf("\tafter seek: %ld\n", tell);

    fseek(volume_info->disk, 36, SEEK_SET);
    fwrite(&(volume_info->ebr->fat_sz_32), sizeof(uint32_t), 1, volume_info->disk);
    fwrite(&(volume_info->ebr->ext_flags), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->ebr->fs_ver), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->ebr->root_clus), sizeof(uint32_t), 1, volume_info->disk);
    fwrite(&(volume_info->ebr->fs_info), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(&(volume_info->ebr->bk_boot_sec), sizeof(uint16_t), 1, volume_info->disk);
    fwrite(volume_info->ebr->reserved, sizeof(uint8_t), 12, volume_info->disk);
    fwrite(&(volume_info->ebr->drv_num), sizeof(uint8_t), 1, volume_info->disk);
    fwrite(&(volume_info->ebr->reserved1), sizeof(uint8_t), 1, volume_info->disk);
    fwrite(&(volume_info->ebr->boot_sig), sizeof(uint8_t), 1, volume_info->disk);
    fwrite(&(volume_info->ebr->vol_id), sizeof(uint32_t), 1, volume_info->disk);
    fwrite(volume_info->ebr->vol_lab, sizeof(uint8_t), 11, volume_info->disk);
    fwrite(volume_info->ebr->fil_sys_type, sizeof(uint8_t), 8, volume_info->disk);

    /* End of sector signature. */
    fseek(volume_info->disk, 510, SEEK_SET);
    fputc(0x55, volume_info->disk);
    fputc(0xAA, volume_info->disk);
}


/****************************************************************
 * Write the FSInfo sector to the volume.
 * Total size: 512 bytes
 */
void write_fsinfo(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);
    long int tell = 0;
    fseek(volume_info->disk, 512, SEEK_SET);
    tell = ftell(volume_info->disk);
    printf("\tafter seek: %ld\n", tell);

    fwrite(volume_info->fsinfo->sector_sig, sizeof(uint8_t), 4, volume_info->disk);

    int i;
    for (i = 0; i < 480; ++i)
        fputc(0x00, volume_info->disk);

    fwrite(volume_info->fsinfo->sector_sig_2, sizeof(uint8_t), 4, volume_info->disk);
    fwrite(&(volume_info->fsinfo->free_clusters), sizeof(uint32_t), 1, volume_info->disk);
    fwrite(&(volume_info->fsinfo->rec_alloc_clus), sizeof(uint32_t), 1, volume_info->disk);
    fwrite(volume_info->fsinfo->reserved, sizeof(uint8_t), 12, volume_info->disk);
    fwrite(volume_info->fsinfo->sector_sig_3, sizeof(uint8_t), 4, volume_info->disk);
}


/****************************************************************
 * Write reserved sectors to the volume.
 */
void write_rsvd(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);
    long int tell = 0;
    fseek(volume_info->disk, 1024, SEEK_SET);
    tell = ftell(volume_info->disk);
    printf("\tafter seek: %ld\n", tell);

    uint32_t pad = (volume_info->bpb->rsvd_sec_cnt *
         volume_info->bpb->bytes_per_sec) - 1024;
    printf("\tpad: %u\n", pad);

    /* Write blank sectors to volume. */
    int i;
    for (i = 0; i < pad; ++i)
        fputc(0x00, volume_info->disk);
}


/****************************************************************
 * Write the File Allocation Table (FAT) to the volume.
 */
void write_fat(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);
    long int tell = 0;
    fseek(volume_info->disk, volume_info->bpb->rsvd_sec_cnt *
        volume_info->bpb->bytes_per_sec, SEEK_SET);
    tell = ftell(volume_info->disk);
    printf("\tafter seek: %ld\n", tell);

    volume_info->fat[(volume_info->bpb->tot_sec_32 / volume_info->bpb->sectors_per_cluster) - 1] = 0xDEADCAFE;

    /* Jump to the first non-reserved sector and write the FAT. */
    fwrite(volume_info->fat, sizeof(uint32_t),
        volume_info->bpb->tot_sec_32 / volume_info->bpb->sectors_per_cluster,
        volume_info->disk);

    /* Write the back-up copy of the FAT. */
    fwrite(volume_info->fat, sizeof(uint32_t),
        volume_info->bpb->tot_sec_32 / volume_info->bpb->sectors_per_cluster,
        volume_info->disk);
}


/****************************************************************
 * Perform all necessary actions to create a new FAT32 volume.
 */
vol_t* create_fs(options_t *opt)
{
    printf("%s\n", __FUNCTION__);

    vol_t *volume_info = (vol_t*) malloc(sizeof(vol_t));
    if (!volume_info)
        return NULL;
    volume_info->bpb    = init_bpb(opt);
    volume_info->ebr    = init_ebr(volume_info);
    volume_info->fsinfo = init_fsinfo(volume_info);
    volume_info->fat    = init_fat(volume_info);
    volume_info         = calc_volume_info(volume_info);

    volume_info->disk = fopen(opt->file_name, "r+");
    if (!volume_info->disk)
        return free_vol(volume_info);

    fseek(volume_info->disk, 0, SEEK_SET);

    write_bpb(volume_info);
    write_ebr(volume_info);
    write_fsinfo(volume_info);
    write_rsvd(volume_info);
    write_fat(volume_info);

    return volume_info;
}


bpb_t* read_bpb(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);

    fseek(volume_info->disk, 0, SEEK_SET);

    bpb_t *bpb = (bpb_t *) malloc(sizeof(bpb_t));
    if (!bpb) return NULL;

    if (fread(bpb, sizeof(bpb_t), 1, volume_info->disk) < 1) {
        free(bpb);
        bpb = NULL;
    }

    return bpb;
}


ebr_t* read_ebr(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);

    fseek(volume_info->disk, 36, SEEK_SET);

    ebr_t *ebr = (ebr_t *) malloc(sizeof(ebr_t));
    if (!ebr) return NULL;

    if (fread(ebr, sizeof(ebr_t), 1, volume_info->disk) < 1) {
        free(ebr);
        ebr = NULL;
    }

    return ebr;
}


fsinfo_t* read_fsinfo(vol_t *volume_info)
{
    printf("%s\n", __FUNCTION__);

    fseek(volume_info->disk, 992, SEEK_SET);

    fsinfo_t *fsinfo = (fsinfo_t *) malloc(sizeof(fsinfo_t));
    if (!fsinfo) return NULL;

    if (fread(fsinfo, sizeof(fsinfo_t), 1, volume_info->disk) < 1) {
        free(fsinfo);
        fsinfo = NULL;
    }

    return fsinfo;
}


fat_t* read_fat(vol_t *volume_info)
{
    int clusters = volume_info->bpb->tot_sec_32 /
        volume_info->bpb->sectors_per_cluster;
    fseek(volume_info->disk,
        (volume_info->bpb->rsvd_sec_cnt *
         volume_info->bpb->bytes_per_sec),
        SEEK_SET);
    fread(volume_info->fat, sizeof(uint32_t), clusters, volume_info->disk);
    fseek(volume_info->disk, clusters, SEEK_CUR);

    return volume_info->fat;
}

void hex_dump_fat(vol_t *volume_info)
{
    int i;
    int clusters =
        volume_info->bpb->tot_sec_32 / volume_info->bpb->sectors_per_cluster;

    printf("%s\n", __FUNCTION__);

    printf("%08x: ", 0);
    for (i = 0; i < clusters; ++i) {
        printf("%08x ", volume_info->fat[i]);
        if ((i + 1) % 16 == 0 && ((i + 1) < clusters)) {
            printf("\n");
            printf("%08x: ", (i + 1));
        }
    }
    printf("\n");
    printf("\tclusters: %d\n", clusters);
}

/****************************************************************
 * Read information (BPB, EBR, FS INFO, FAT) from a FAT32 volume.
 *
 */
vol_t* read_fs(options_t *opt)
{
    printf("%s\n", __FUNCTION__);


    vol_t *volume_info = (vol_t *) malloc(sizeof(vol_t));
    if (!volume_info) return NULL;

    volume_info->disk = fopen(opt->file_name, "r+");
    if (volume_info->disk == NULL)
        return free_vol(volume_info);

    volume_info->bpb = read_bpb(volume_info);
    volume_info->ebr = read_ebr(volume_info);
    volume_info->fsinfo = read_fsinfo(volume_info);
    volume_info->fat = init_fat(volume_info);
    volume_info->fat = read_fat(volume_info);
    volume_info = calc_volume_info(volume_info);

    printf("FAT\n");
    hex_dump_fat(volume_info);

    /* Print BPB */
    printf("BPB\n");
    printf("\tbs_jmp_boot: %02x %02x %02x\n", volume_info->bpb->bs_jmp_boot[0],
        volume_info->bpb->bs_jmp_boot[1], volume_info->bpb->bs_jmp_boot[2]);
    printf("\tbs_oem_name: %s\n", volume_info->bpb->bs_oem_name);
    printf("\tbytes_per_sec: %d\n", volume_info->bpb->bytes_per_sec);
    printf("\tsectors_per_cluster: %d\n",
        volume_info->bpb->sectors_per_cluster);
    printf("\trsvd_sec_cnt: %d\n", volume_info->bpb->rsvd_sec_cnt);
    printf("\tnum_fats: %d\n", volume_info->bpb->num_fats);
    printf("\troot_ent_cnt: %d\n", volume_info->bpb->root_ent_cnt);
    printf("\ttot_sec_16: %d\n", volume_info->bpb->tot_sec_16);
    printf("\tmedia: %02x\n", volume_info->bpb->media);
    printf("\tfat_sz_16: %04x\n", volume_info->bpb->fat_sz_16);
    printf("\tsec_per_trk: %04x\n", volume_info->bpb->sec_per_trk);
    printf("\tnum_heads: %04x\n", volume_info->bpb->num_heads);
    printf("\thidd_sec: %08x\n", volume_info->bpb->hidd_sec);
    printf("\ttot_sec_32: %d\n\n", volume_info->bpb->tot_sec_32);


    return volume_info;
}


vol_t* free_vol(vol_t *volume_info)
{
    if (volume_info != NULL) {
        fclose(volume_info->disk);
        free(volume_info->bpb);
        free(volume_info->ebr);
        free(volume_info->fsinfo);
        free(volume_info->fat);
        free(volume_info);
    }

    return NULL;
}

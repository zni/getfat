/****************************************************************
 * FAT32 driver for volume creation.
 *
 * Author : Matt Godshall
 * Date   : 2011 Nov 12 21:06:27
 *
 * This file is part of getfat.
 *
 * getfat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * getfat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with getfat.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log.h>
#include <fat32.h>


/****************************************************************
 * Look up the n-th cluster.
 */
PRIVATE u32_t lookup_cluster(u32_t cluster)
{
    return 0;
}

/****************************************************************
 * Allocate and initialize a BIOS Parameter Block.
 */
PRIVATE bpb_t* init_bpb(options_t *opt)
{
    bpb_t *bpb = (bpb_t *) malloc(sizeof(bpb_t));

    if (!bpb) error(FATAL, 0, "Could not allocate space for BPB.\n");

    bpb->bs_jmp_boot[0] = 0x6B;
    bpb->bs_jmp_boot[1] = 0x3C;
    bpb->bs_jmp_boot[2] = 0x90;

    bpb->bs_oem_name[0] = 'M';
    bpb->bs_oem_name[1] = 'S';
    bpb->bs_oem_name[2] = 'W';
    bpb->bs_oem_name[3] = 'I';
    bpb->bs_oem_name[4] = 'N';
    bpb->bs_oem_name[5] = '4';
    bpb->bs_oem_name[6] = '.';
    bpb->bs_oem_name[7] = '1';

    // Variable
    bpb->bytes_per_sec       = 512;
    bpb->sectors_per_cluster = 2;

    // Mostly fixed.
    bpb->rsvd_sec_cnt = 32;
    bpb->num_fats     = 2;
    bpb->root_ent_cnt = 0;
    bpb->tot_sec_16   = 0;
    bpb->media        = 0xF8;
    bpb->fat_sz_16    = 0;
    bpb->sec_per_trk  = 0;
    bpb->num_heads    = 0;
    bpb->hidd_sec     = 0;

    // 2 * bytes_per_sec = 1024 = 1KB
    // 1KB * 1024               = 1MB
    // device_size is in megabytes.
    bpb->tot_sec_32   = opt->device_size * ((bpb->bytes_per_sec * 2) * 2);

    return bpb;
}


/****************************************************************
 * Allocate and initialize the Extended Boot Record.
 */
PRIVATE ebr_t* init_ebr(bpb_t *bpb)
{
    ebr_t *ebr = (ebr_t *) malloc(sizeof(ebr_t));

    if (!ebr) error(FATAL, 0, "Could not allocate space for EBR.\n");

    ebr->fat_sz_32   = bpb->tot_sec_32;
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
    strncpy(ebr->vol_lab, "NO NAME    ", 11);
    strncpy(ebr->fil_sys_type, "FAT32   ", 8);

    return ebr;
}


/****************************************************************
 * Allocate and initialize the File System Info Block.
 */
PRIVATE fsinfo_t* init_fsinfo(bpb_t *bpb)
{
    fsinfo_t *fsinfo = (fsinfo_t *) malloc(sizeof(fsinfo_t));

    if (!fsinfo) error(FATAL, 0, "Could not allocate space for FSINFO.\n");

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
PRIVATE fat_t* init_fat(bpb_t *bpb)
{
    fat_t *fat = (fat_t*) calloc((bpb->tot_sec_32 / 2), sizeof(fat_t));
    if (!fat) error(FATAL, 0, "Could not allocate space for the FAT.\n");

    /* memset(fat, FREE, sizeof(fat_t) * (bpb->tot_sec_32 / 2)); */

    return fat;
}


/****************************************************************
 * Allocate and initialize the volume information.
 *
 * vol_t provides information on the volume, such as:
 * the LBA of the FAT, the LBA of the first cluster,
 * and other fun things.
 */
PRIVATE vol_t* init_vol(bpb_t *bpb, ebr_t *ebr)
{
    vol_t *vol = (vol_t*) malloc(sizeof(vol_t));

    if (!vol) error(FATAL, 0, "Could not allocate space for Volume Info.\n");
    
    static const u8_t PARTITION_BEGIN_LBA = 0;

    vol->fat_begin_lba = PARTITION_BEGIN_LBA + bpb->rsvd_sec_cnt;
    vol->cluster_begin_lba = PARTITION_BEGIN_LBA + bpb->rsvd_sec_cnt +
        (bpb->num_fats * bpb->tot_sec_32);
    vol->sectors_per_cluster = bpb->sectors_per_cluster;
    vol->root_dir_first_cluster = ebr->root_clus;

    return vol;
}


/****************************************************************
 * Write the BIOS Parameter Block to the volume.
 */
PRIVATE void write_bpb(FILE *f, bpb_t *bpb)
{
    printf("write_bpb\n");
    long int tell = 0;
    tell = ftell(f);
    printf("\tbefore seek: %ld\n", tell);
    fseek(f, 0, SEEK_SET);
    tell = ftell(f);
    printf("\tafter seek: %ld\n", tell);


    fwrite(bpb->bs_jmp_boot, sizeof(u8_t), 3, f);
    fwrite(bpb->bs_oem_name, sizeof(u8_t), 8, f);
    fwrite(&(bpb->bytes_per_sec), sizeof(u16_t), 1, f);
    fwrite(&(bpb->sectors_per_cluster), sizeof(u8_t), 1, f);
    fwrite(&(bpb->rsvd_sec_cnt), sizeof(u16_t), 1, f);
    fwrite(&(bpb->num_fats), sizeof(u8_t), 1, f);
    fwrite(&(bpb->root_ent_cnt), sizeof(u16_t), 1, f);
    fwrite(&(bpb->tot_sec_16), sizeof(u16_t), 1, f);
    fwrite(&(bpb->media), sizeof(u8_t), 1, f);
    fwrite(&(bpb->fat_sz_16), sizeof(u16_t), 1, f);
    fwrite(&(bpb->sec_per_trk), sizeof(u16_t), 1, f);
    fwrite(&(bpb->num_heads), sizeof(u16_t), 1, f);
    fwrite(&(bpb->hidd_sec), sizeof(u32_t), 1, f);
    fwrite(&(bpb->tot_sec_32), sizeof(u32_t), 1, f);
}


/****************************************************************
 * Write the Extended Boot Record to the volume. 
 */
PRIVATE void write_ebr(FILE *f, ebr_t *ebr)
{
    printf("write_ebr\n");
    long int tell = 0;
    tell = ftell(f);
    printf("\tbefore seek: %ld\n", tell);
    fseek(f, 36, SEEK_SET);
    tell = ftell(f);
    printf("\tafter seek: %ld\n", tell);

    fseek(f, 36, SEEK_SET);
    fwrite(&(ebr->fat_sz_32), sizeof(u32_t), 1, f);
    fwrite(&(ebr->ext_flags), sizeof(u16_t), 1, f);
    fwrite(&(ebr->fs_ver), sizeof(u16_t), 1, f);
    fwrite(&(ebr->root_clus), sizeof(u32_t), 1, f);
    fwrite(&(ebr->fs_info), sizeof(u16_t), 1, f);
    fwrite(&(ebr->bk_boot_sec), sizeof(u16_t), 1, f);
    fwrite(ebr->reserved, sizeof(u8_t), 12, f);
    fwrite(&(ebr->drv_num), sizeof(u8_t), 1, f);
    fwrite(&(ebr->reserved1), sizeof(u8_t), 1, f);
    fwrite(&(ebr->boot_sig), sizeof(u8_t), 1, f);
    fwrite(&(ebr->vol_id), sizeof(u32_t), 1, f);
    fwrite(ebr->vol_lab, sizeof(u8_t), 11, f);
    fwrite(ebr->fil_sys_type, sizeof(u8_t), 8, f);

    /* End of sector signature. */
    fseek(f, 510, SEEK_SET);
    fputc(0x55, f);
    fputc(0xAA, f);
}


u32_t read_sector_sig(FILE *f)
{
    u32_t sector_sig = 0;
    fseek(f, 508, SEEK_SET);
    fread(&sector_sig, sizeof(u32_t), 1, f);
    return sector_sig;
}


/****************************************************************
 * Write the FSInfo sector to the volume.
 */
PRIVATE void write_fsinfo(FILE *f, fsinfo_t *fsinfo)
{
    printf("write_fsinfo\n");
    long int tell = 0;
    tell = ftell(f);
    printf("\tbefore seek: %ld\n", tell);
    fseek(f, 512, SEEK_SET);
    tell = ftell(f);
    printf("\tafter seek: %ld\n", tell);

    fwrite(fsinfo->sector_sig, sizeof(u8_t), 4, f);

    int i;
    for (i = 0; i < 480; ++i)
        fputc(0x00, f); 

    fwrite(fsinfo->sector_sig_2, sizeof(u8_t), 4, f);
    fwrite(&(fsinfo->free_clusters), sizeof(u32_t), 1, f);
    fwrite(&(fsinfo->rec_alloc_clus), sizeof(u32_t), 1, f);
    fwrite(fsinfo->reserved, sizeof(u8_t), 12, f);
    fwrite(fsinfo->sector_sig_3, sizeof(u8_t), 4, f);
}


/****************************************************************
 * Write reserved sectors to the volume.
 */
PRIVATE void write_rsvd(FILE *f, bpb_t *bpb)
{
    u16_t rsvd_sec_cnt = bpb->rsvd_sec_cnt;

    /* Write blank sectors to volume. */
    int i, j;
    for (i = 0; i < rsvd_sec_cnt; ++i) {
        for (j = 0; j < bpb->bytes_per_sec; ++j) {
            fputc(0x00, f);
        }
    }
}


/****************************************************************
 * Write the File Allocation Table (FAT) to the volume.
 */
PRIVATE void write_fat(FILE *f, bpb_t *bpb, fat_t *fat)
{
    /* Jump to the first non-reserved sector and write the FAT. */
    fseek(f, bpb->rsvd_sec_cnt * bpb->bytes_per_sec, SEEK_SET);
    fwrite(fat, bpb->tot_sec_32 / 2, 1, f);

    /* Write the back-up copy of the FAT. */
    fwrite(fat, bpb->tot_sec_32 / 2, 1, f);
}


/****************************************************************
 * Perform all necessary actions to create a new FAT32 volume.
 */
void create_fs(options_t *opt)
{
    bpb_t *bpb       = init_bpb(opt);
    ebr_t *ebr       = init_ebr(bpb);
    fsinfo_t *fsinfo = init_fsinfo(bpb);
    fat_t *fat       = init_fat(bpb);
    vol_t *vol       = init_vol(bpb, ebr);

    FILE *volume = fopen(opt->file_name, "w+");

    if (!volume)
        error(FATAL, 0, "An error occured opening the volume for writing.\n");

    write_bpb(volume, bpb);
    write_ebr(volume, ebr);
    write_fsinfo(volume, fsinfo);
    write_rsvd(volume, bpb);
    write_fat(volume, bpb, fat);


    fclose(volume);

    free(bpb);
    bpb = NULL;
    free(ebr);
    ebr = NULL;
    free(fsinfo);
    fsinfo = NULL;
    free(fat);
    fat = NULL;
    free(vol);
    vol = NULL;
}


/****************************************************************
 * Read information (BPB, EBR, FS INFO, FAT) from a FAT32 volume.
 *
 * XXX Debugging function.
 */
void read_fs(options_t *opt)
{
    FILE *volume = fopen(opt->file_name, "r+");
    printf("read_fs\n");
    printf("\tboot signature: %08x\n", read_sector_sig(volume));

    /*
    bpb_t *bpb = read_bpb(volume);
    ebr_t *ebr = read_ebr(volume);
    fsinfo_t *fsinfo = read_fsinfo(volume);
    fat_t *fat = read_fat(volume);
    */
}

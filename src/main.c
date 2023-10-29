/****************************************************************
 * Volume formatter for getfat utilities.
 *
 * Author: Matt Godshall <lifeinhex@gmail.com>
 * Date  : 06-25-2012
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fat32.h>
#include <options.h>

options_t* parse_args(int, char**);
void display_help();
void find_first_file(vol_t *);


int main(int argc, char **argv)
{
    vol_t *volume_info = NULL;
    options_t *args;
    args = parse_args(argc, argv);
    if (!args) return 1;

    if (args->help)
        display_help();
    else if (args->create && args->file_name)
        volume_info = create_fs(args);
    else if (args->read && args->file_name) {
        volume_info = read_fs(args);
        find_first_file(volume_info);
    } else
        display_help();

    free(args->file_name);
    args->file_name = NULL;
    free(args);
    args = NULL;

    free_vol(volume_info);

    return 0;
}


/****************************************************************
 * Parse command line arguments.
 */
options_t* parse_args(int argc, char **argv)
{
    options_t *args = malloc(sizeof(options_t));
    if (!args) return NULL;

    args->device_size = 0;
    args->cluster_size = 4;
    args->reserved_sectors = 2;
    args->sector_size = 512;
    args->file_name = NULL;
    args->create = 0;
    args->read = 0;
    args->help = 0;
    
    int i;
    for (i = 1; i < argc; ++i) {
        char *str = argv[i];
        
        /* Strip hyphens */
        if (*str == '-')
            ++str;
        
        /* Device Size */
        if (!strncmp("ds", str, 2)) {
            str += 2;
            args->device_size = atoi(str);

        /* Help */
        } else if (!strncmp("h", str, 1)) {
            args->help = 1;

        /* Create */
        } else if (!strncmp("c", str, 1)) {
            args->create = 1;

        /* Read */
        } else if (!strncmp("r", str, 1)) {
            args->read = 1;

        /* Sector Size */
        } else if (!strncmp("ss", str, 2)) {
            str += 2;
            args->sector_size = atoi(str);

        /* Cluster Size */
        } else if (!strncmp("cs", str, 2)) {
            str += 2;
            args->cluster_size = atoi(str);

        /* Reserved Sectors */
        } else if (!strncmp("rs", str, 2)) {
            str += 2;
            args->reserved_sectors = atoi(str);

        /* Filename */
        } else if (!strncmp("f", str, 1)) {
            uint16_t len = strlen(++str) + 1;
            /* Check if filename is present. */
            if (len == 1) {
                args->help = 1;
                return args;
            } else {
                args->file_name = (char *) malloc(len);
                strncpy(args->file_name, str, len);
            }
        } else {
            args->help = 1;
            break;
        }
    }

    return args;
}


/****************************************************************
 * Display help message.
 */
void display_help()
{
    fprintf(stdout, "GetFat v0.0.1 - a FAT32 volume creator.\n");
    fprintf(stdout, "getfat -r|-c <options>\n");
    fprintf(stdout, "\t-ds<size>\tspecify device size in megabytes\n");
    fprintf(stdout, "\t-h\t\tdisplay this text\n");
    fprintf(stdout, "\t-c\t\tcreate the volume\n");
    fprintf(stdout, "\t-r\t\tread the volume\n");
    fprintf(stdout, "\t-f<name>\tread the given file\n");
    fprintf(stdout, "\t-ss<size>\tsector size\n");
    fprintf(stdout, "\t-cs<size>\tclusters per sector\n");
}

void find_first_file(vol_t *volume_info)
{
    int i, j;

    int clusters = volume_info->bpb->tot_sec_32 /
        volume_info->bpb->sectors_per_cluster;

    int fat_size = volume_info->ebr->fat_sz_32 * volume_info->bpb->bytes_per_sec;

    int cluster_size = volume_info->bpb->bytes_per_sec *
        volume_info->bpb->sectors_per_cluster;

    int first_data_cluster = volume_info->bpb->tot_sec_32 / cluster_size;

    int rsvd_size = volume_info->bpb->rsvd_sec_cnt *
        volume_info->bpb->bytes_per_sec;

    uint8_t *first_bytes = (uint8_t*) calloc(64, sizeof(uint8_t));

    printf("%s\n", __FUNCTION__);
    printf("\tskipping to: %08x\n", rsvd_size + fat_size);

    fseek(volume_info->disk, rsvd_size + fat_size, SEEK_SET);
    fread(first_bytes, sizeof(uint8_t), 64, volume_info->disk);

    for (i = 0; i < 64; ++i) {
        printf("%02x ", first_bytes[i]);

        if ((i + 1) % 16 == 0) {
            putchar('\t');
            for (j = (i + 1) - 16; j < i; ++j) {
                if (isalnum(first_bytes[j])) {
                    printf("%c ", first_bytes[j]);
                } else {
                    printf(". ");
                }
            }
            putchar('\n');
        }
    }

    free(first_bytes);
}

/****************************************************************
 * Driver for getfat.
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


int main(int argc, char **argv)
{
    options_t *args;
    args = parse_args(argc, argv);
    if (!args) return 1;

    if (args->create && args->file_name)
        create_fs(args);
    else if (args->read && args->file_name)
        read_fs(args);
    else
        display_help();

    free(args->file_name);
    args->file_name = NULL;

    free(args);
    args = NULL;

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
    args->sector_size = 512;
    args->file_name = NULL;
    args->create = 0;
    args->read = 0;
    
    int i;
    for (i = 1; i < argc; ++i) {
        char *str = argv[i];
        
        if (*str == '-')
            ++str;
        
        /* Parse device_size. */
        if (!strncmp("ds", str, 2)) {
            str += 2;
            args->device_size = atoi(str);

        } else if (!strncmp("h", str, 1)) {
            display_help();

        } else if (!strncmp("c", str, 1)) {
            args->create = 1;

        } else if (!strncmp("r", str, 1)) {
            args->read = 1;

        } else if (!strncmp("ss", str, 2)) {
            str += 2;
            args->sector_size = atoi(str);

        /* TODO Exit if filename is not present. */
        } else if (!strncmp("f", str, 1)) {
            u16_t len = strlen(++str) + 1;
            args->file_name = (char *) malloc(len); 
            strncpy(args->file_name, str, len);

        } else {
            display_help();
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
    fprintf(stdout, "GetFat v0.0.1\n");
    fprintf(stdout, "\t-ds<size>\tspecify device size in megabytes\n");
    fprintf(stdout, "\t-h\t\tdisplay this text\n");
    fprintf(stdout, "\t-c\t\tcreate the volume\n");
    fprintf(stdout, "\t-f<name>\tread the given file\n");
}

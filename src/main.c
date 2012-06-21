#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fat32.h"
#include "options.h"

options_t* parse_args(int, char**);
void display_help();


int main(int argc, char **argv)
{
    options_t *args;
    args = parse_args(argc, argv);

    if (args->create)
        create_fs(args);

    return 0;
}


/*
 * Parse command line arguments.
 */
options_t* parse_args(int argc, char **argv)
{
    options_t *args = malloc(sizeof(options_t));
    int i;

    args->create = 0;
    
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


/*
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

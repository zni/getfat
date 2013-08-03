#ifndef OPTIONS_H
#define OPTIONS_H

#include "types.h"

typedef struct _options {
    u32_t  device_size;
    u32_t  sector_size;
    u32_t  cluster_size;
    u32_t  reserved_sectors;
     i8_t* file_name;
     u8_t  create;
     u8_t  read;
     u8_t  help;
} options_t;
#endif

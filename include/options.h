#ifndef OPTIONS_H
#define OPTIONS_H

#include "types.h"

typedef struct _options {
    uint32_t  device_size;
    uint32_t  sector_size;
    uint32_t  cluster_size;
    uint32_t  reserved_sectors;
    char*     file_name;
    uint8_t   create;
    uint8_t   read;
    uint8_t   help;
} options_t;
#endif

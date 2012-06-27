#ifndef OPTIONS_H
#define OPTIONS_H

#include "types.h"

typedef struct _options {
    u32_t  device_size;
    u32_t  sector_size;
     i8_t* file_name;
     u8_t  create;
     u8_t  read;
} options_t;
#endif

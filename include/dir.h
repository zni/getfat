#ifndef DIR_H
#define DIR_H

#include "types.h"

typedef enum attrib {
    READ_ONLY = 0x01,
    HIDDEN    = 0x02,
    SYSTEM    = 0x04,
    VOLUME_ID = 0x08,
    DIRECTORY = 0x10,
    ARCHIVE   = 0x20,
    LONG      = 0x0F
} attrib_t;

typedef struct _dir {
    /* 
     * If the file name is shorter than 8 bytes, the unused bytes are
     * filled with spaces (0x20).
     */
    uint8_t name[11];    // 11 bytes

    /*
     * Attribute byte
     * --------------
     * 0 (LSB) - read only - 1 - should not allow writing
     * 1       - hidden    - 1 - should not show in dir listing
     * 2       - system    - 1 - file is operating system
     * 3       - volume id - 1 - filename is volume id
     * 4       - directory - x - is a subdirectory (32-byte records)
     * 5       - archive   - x - has been changed since last backup
     * 6       - unused    - 0 - should be zero
     * 7 (MSB) - unused    - 0 - should be zero
     */
    attrib_t  attr;

    /* Set to 0 on creation, never use again. */
    uint8_t  nt_res; 

    /* Millisecond stamp at file creation time. */
    uint8_t  crt_time_tenth;

    /* Time file was created. */
    uint16_t crt_time;
    
    /* Date file was created. */
    uint16_t crt_date;

    /* Last access date. (Set to wrt_date) */
    uint16_t lst_acc_date;

    /* High word of entry's first cluster number. */
    uint16_t fst_clus_hi;

    /* Time of last write. */
    uint16_t wrt_time;

    /* Date of last write. */
    uint16_t wrt_date;

    /* Low word of this entry's first cluster number. */
    uint16_t fst_clus_lo;

    uint32_t file_size;
}__attribute__((packed)) dir_t;
#endif

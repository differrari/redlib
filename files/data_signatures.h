#pragma once

#include "types.h"

#define DATA_SIGNATURE(arr) (((uint64_t)(((uint8_t *)(arr))[0]) <<  0)+\
                            ((uint64_t)(((uint8_t *)(arr))[1]) <<  8)+\
                            ((uint64_t)(((uint8_t *)(arr))[2]) << 16)+\
                            ((uint64_t)(((uint8_t *)(arr))[3]) << 24)+\
                            ((uint64_t)(((uint8_t *)(arr))[4]) << 32)+\
                            ((uint64_t)(((uint8_t *)(arr))[5]) << 40)+\
                            ((uint64_t)(((uint8_t *)(arr))[6]) << 48)+\
                            ((uint64_t)(((uint8_t *)(arr))[7]) << 56))

typedef u64 data_signature;

#define DATA_SIG_UNKNOWN 0

#define DATA_SIG_CMD DATA_SIGNATURE("CMD")
#define DATA_SIG_TEXT DATA_SIGNATURE("TEXT")
#define DATA_SIG_REDPKG DATA_SIGNATURE("REDPKG")
#define DATA_SIG_RAW DATA_SIGNATURE("RAWDATA")

#define DATA_SIG_THEME DATA_SIGNATURE("REDTHEME")
#define DATA_SIG_PROC_ST DATA_SIGNATURE("PROCST")

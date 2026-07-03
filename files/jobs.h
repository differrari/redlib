#pragma once

#include "types.h"

typedef u64 job_id_t;

typedef enum {
    job_none,
    job_read,
    job_write,
    job_stat 
} job_types;

typedef struct {
    uptr requesting_tid;
    uptr requesting_pid;
    u16 worker_pid;
    job_types type;
    //buffers
} job_application_t;
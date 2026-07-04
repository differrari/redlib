#pragma once

#include "types.h"
#include "files/fs.h"

typedef u64 job_id_t;

typedef enum {
    job_none,
    // job_read,
    // job_write,
    job_stat 
} job_types;

#define MAX_JOB_BUFFERS 8

typedef struct {
    sizedptr ptr;
    u8 arg_num;
    bool explicit_size;
} job_buffer;

typedef struct {
    uptr requesting_tid;
    uptr requesting_pid;
    u16 worker_pid;
    job_types type;
    file fd;
    job_buffer buffers[8];
    size_t buffer_count;
    //buffers
} job_application_t;
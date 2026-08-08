#pragma once

#include "types.h"
#include "files/fs.h"

typedef u64 job_id_t;

typedef enum {
    job_none,
    job_open,
    job_read,
    job_write,
    job_trunc,
    job_close,
    job_stat,
    job_readdir,
    job_transform,
} job_types;

#define MAX_JOB_BUFFERS 8

typedef enum { copy_on_start = 1 << 0, copy_on_end = 1 << 1 } job_sync_type;

typedef struct {
    sizedptr worker_ptr;
    sizedptr orig_ptr;
    u8 arg_num;
    bool explicit_size;
    job_sync_type sync;
    bool fd;
} job_buffer;

typedef struct {
    uptr requesting_tid;
    uptr requesting_pid;
    u16 worker_pid;
    job_types type;
    job_buffer buffers[8];
    size_t buffer_count;
    //buffers
} job_application_t;
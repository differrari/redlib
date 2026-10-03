#pragma once

#include "types.h"

#define MAX_PROC_NAME_LENGTH 256

typedef struct {
    u16 id;
    u8 procnamelen;
    char procname[MAX_PROC_NAME_LENGTH];
    sizedptr stack;
    uptr sp;
    sizedptr heap;
    uptr pc;
    bool privilege;
    u32 state;
    // i64 entitlement_count;
    // i64 files_count;
} proc_info;

typedef enum { PROC_STOPPED, PROC_READY, PROC_RUNNING, PROC_BLOCKED, PROC_SLEEPING } process_state_new;//TODO: use these in process.h
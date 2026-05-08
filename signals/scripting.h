#pragma once

#include "shell/sheldon/shelley.h"
#include "string.h"

SHELLEY_CMD(kill, {
    SHELLEY_GET_ARG(pid);
    i64 proc_id = parse_int_u64(pid, 5);
    send_signal(SIG_KILL, proc_id);
    return exit_return_success;
}, SHELLEY_ARG_POS(pid, false, 0));
#include "shelley.h"
#include "data/struct/hashmap.h"
#include "syscalls/syscalls.h"
#include "files/scripting.h"
#include "sheldon.h"

SHELLEY_CMD_FWD_1ARG(echo, print, string);

extern void register_builtin(shell_handle *handle, char *name, cmd_def *def);

#define REG_BUILTIN(fn) register_builtin(handle, #fn,&fn##_def);

void register_sheldon_builtins(shell_handle *handle){
    REG_BUILTIN(echo);
    REG_BUILTIN(pwd);
    REG_BUILTIN(read);
    REG_BUILTIN(ls);
    REG_BUILTIN(cd);
}
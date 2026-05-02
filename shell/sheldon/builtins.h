#include "shelley.h"
#include "data/struct/hashmap.h"
#include "syscalls/syscalls.h"
#include "files/scripting.h"
#include "sheldon.h"

extern hash_map_t *sheldon_builtin_hashmap;

SHELLEY_CMD(echo, 1, { SHELLEY_ARG_POS_ALL(string, false, 0) }, {
    SHELLEY_GET_ARG(string);
    shell_put(handle, "%s", string);
    return exit_return_success;
});

extern void register_builtin(char *name, cmd_def *def);

#define REG_BUILTIN(fn) register_builtin(#fn,&fn##_def);

void register_sheldon_builtins(){
    sheldon_builtin_hashmap = hash_map_create(256);
    REG_BUILTIN(echo);
    REG_BUILTIN(pwd);
    REG_BUILTIN(read);
    REG_BUILTIN(ls);
}
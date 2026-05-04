#include "sheldon.h"
#include "builtins.h"
#include "builtin_handler.h"

void sheldon_init(shell_handle *handle){
    shell_print(handle, "$heldon");
}

bool sheldon_run_cmd(shell_handle *handle, string_slice fullcmd){
    string_splitter splitter = make_string_splitter_slice(fullcmd, ' ', false);
    if (!string_splitter_advance(&splitter))
        return false;
    string_slice cmd = string_splitter_get_current(&splitter);
    if (call_sheldon_builtin(handle, cmd, string_splitter_remaining(&splitter), 0)) return true;
    //Call exec
    return false;
}

shell_handle* create_sheldon(shell_bindings bindings, void (*register_builtins)(shell_handle *handle)){
    shell_handle *handle = zalloc(sizeof(shell_handle) + sizeof(shell_ctx) + sizeof(sheldon_ctx));
    shell_ctx *shctx = (shell_ctx*)((uptr)handle + sizeof(shell_handle));
    sheldon_ctx* loctx = (sheldon_ctx*)((uptr)shctx + sizeof(shell_ctx));
    handle->local_ctx = loctx;
    handle->common_ctx = shctx;
    loctx->builtins = hash_map_create(256);
    if (!register_builtins) register_builtins = register_sheldon_builtins;
    register_builtins(handle);
    handle->cmd_input = sheldon_run_cmd;
    new_shell(handle, bindings, sheldon_init);
    return handle;
}
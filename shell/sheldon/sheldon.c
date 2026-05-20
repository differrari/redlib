#include "sheldon.h"
#include "builtins.h"
#include "builtin_handler.h"
#include "environment/env_types.h"
#include "data/serialize/binary_serial.h"
#include "kbd_helper.h"

void sheldon_init(shell_handle *handle){
    shell_print(handle, "$heldon");
}

void sheldon_emit_data(structdef field, sizedptr data, bool is_allocated){
    if (!data.ptr || !data.size) return;
        switch (field.type) {
        case binary_type_i8: print("%S: %i",field.name,*(i8*)data.ptr); break;
        case binary_type_i16: print("%S: %i",field.name,*(i16*)data.ptr); break;
        case binary_type_i32: print("%S: %i",field.name,*(i32*)data.ptr); break;  
        case binary_type_i64: print("%S: %i",field.name,*(i64*)data.ptr); break;
        case binary_type_float: print("%S: %f",field.name,*(float*)data.ptr); break;
        case binary_type_double: print("%S: %f",field.name,*(double*)data.ptr); break;
        case binary_type_string: print("%S: %v",field.name,data); break;
        default: return;
    }
    if (is_allocated) release((void*)data.ptr);
}

bool sheldon_run_cmd(shell_handle *handle, string_slice fullcmd){
    string_splitter splitter = make_string_splitter_slice(fullcmd, ' ', false);
    if (!string_splitter_advance(&splitter))
        return false;
    string_slice cmd = string_splitter_get_current(&splitter);
    if (call_sheldon_builtin(handle, cmd, string_splitter_remaining(&splitter), 0)) return true;
    
    int32_t proc = system_focus(fullcmd.data, EXEC_MODE_KEEP_FOCUS);
    if (!proc) return false;

    string output_string = string_format("/proc/%i/out", proc);
    string state_string = string_format("/proc/%i/state", proc);
    string config_string = string_format("/environments/%i/config", proc);
    string data_string = string_format("/environments/%i/data", proc);
    string structure_string = string_format("/environments/%i/structure", proc);
    
    file out_fd, state_fd, display_fd, data_fd;
    openf(output_string.data, &out_fd);
    string_free(output_string);
    FS_RESULT state_res = openf(state_string.data, &state_fd);
    string_free(state_string);
    openf(config_string.data, &display_fd);
    string_free(config_string);
    openf(data_string.data, &data_fd);
    string_free(data_string);

    binary_serializer proc_serializer = {};
    
    if (state_res != FS_RESULT_SUCCESS){
        print("Failed to open process state");
        return true;
    }

    int state = 1;
    size_t amount = 0x100;
    char *buf = (char*)zalloc(amount + 1);
    if (!buf) {
        closef(&out_fd);
        closef(&state_fd);
        return true;
    }

    char* data_buf = (char*)zalloc(0x1000);

    env_config proc_env_config = {env_display_text,env_behavior_scroll};
    do {
        kbd_event event;
        if (read_event(&event)){
            if (!handle_modifier(&event)){
                char cmd = hid_to_char(event.key, current_modifier);
                if (event.type == KEY_PRESS && handle->bindings.console_ascii_cmd) handle->bindings.console_ascii_cmd(handle, cmd, proc);
            }
        }
        size_t n = readf(&out_fd, buf, amount);
        buf[n] = 0;
        if (n){
            for (size_t i = 0; i < n; i++){
                if (buf[i] == ENV_CMD_CONFIG_SYNC){
                    env_config read_env_config = {};
                    if (!display_fd.id || readf(&display_fd, (char*)&read_env_config, sizeof(env_config)) != sizeof(env_config)) read_env_config = (env_config){env_display_text,env_behavior_scroll};
                    if (read_env_config.behavior != proc_env_config.behavior)
                        proc_env_config.behavior = read_env_config.behavior;
                    if (read_env_config.display_type != proc_env_config.display_type)
                        proc_env_config.display_type = read_env_config.display_type;
                } if (buf[i] == ENV_CMD_STRUCT_SYNC){
                    size_t struct_size = 0;
                    char* struct_buf = read_full_file(structure_string.data, &struct_size);
                    if (struct_size && struct_buf){
                        bin_ser_define_structure(&proc_serializer, struct_buf, struct_size);
                    } else print("Invalid structure");
                } if (buf[i] == ENV_CMD_DATA_SYNC){
                    if (proc_env_config.behavior == env_behavior_wipe && handle->bindings.console_clean) handle->bindings.console_clean(handle);
                    size_t data_size = readf(&data_fd, data_buf, 0x1000);
                    if (data_size && data_buf){
                        if (!bin_ser_deserialize(&proc_serializer, (sizedptr){(uptr)data_buf,data_size}, sheldon_emit_data)) print("Data deserialization failed");
                    } else print("Invalid data");
                } else if (buf[i] == '\a'){
                    if (handle->bindings.console_bell) handle->bindings.console_bell(handle);
                } else if (proc_env_config.display_type == env_display_text && handle->bindings.console_output) handle->bindings.console_output(handle, buf[i]);
            }
            if (handle->bindings.console_flush) handle->bindings.console_flush(handle);
        }

        seek(&state_fd, 0, SEEK_ABSOLUTE);
        if (readf(&state_fd, (char*)&state, sizeof(int)) != sizeof(int)) state = 0;
        // print("Display type %i",proc_display_type);
        // if (state && !n) msleep(20);
    } while (state);

    for (;;) {
        size_t n = readf(&out_fd, buf, amount);
        if (!n) break;
        buf[n] = 0;
        shell_print_specify_newline(handle, false, buf);
    }

    release(buf);
    closef(&out_fd);
    closef(&state_fd);

    string exit_msg = string_format("\nProcess %i ended.", proc);
    // put_string(exit_msg.data);
    string_free(exit_msg);
    
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
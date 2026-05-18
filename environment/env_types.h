#pragma once

#include "syscalls/syscalls.h"

typedef enum { env_display_text, env_display_table, env_display_document } env_display_type;
typedef enum { env_behavior_scroll, env_behavior_wipe } env_behavior;

typedef struct {
    env_display_type display_type;
    env_behavior behavior;
} env_config;

#define ENV_CMD_CONFIG_SYNC_STR "\["
#define ENV_CMD_CONFIG_SYNC (*ENV_CMD_CONFIG_SYNC_STR)
#define ENV_CMD_STRUCT_SYNC_STR "\1"
#define ENV_CMD_STRUCT_SYNC (*ENV_CMD_STRUCT_SYNC_STR)
#define ENV_CMD_DATA_SYNC_STR "\3"
#define ENV_CMD_DATA_SYNC (*ENV_CMD_DATA_SYNC_STR)
#define ENV_CMD_BEHAVIOR_SYNC_STR "\17"
#define ENV_CMD_BEHAVIOR_SYNC (*ENV_CMD_BEHAVIOR_SYNC_STR)

static inline void env_config_sync(){
    print(ENV_CMD_CONFIG_SYNC_STR);
}

static inline void env_struct_sync(){
    print(ENV_CMD_STRUCT_SYNC_STR);
}

static inline void env_data_sync(){
    print(ENV_CMD_DATA_SYNC_STR);
}

static inline void set_environment_config(env_config config){
    swritef("/environment/config", &config, sizeof(config), false);
    env_config_sync();
}

#include "data/serialize/binary_serial.h"

static inline void set_environment_structure(structdef *fields, size_t num){
    sizedptr serialized_struct = bin_ser_emit_structure(fields, num);
    swritef("/environment/structure", (char*)serialized_struct.ptr, serialized_struct.size, false);
    env_struct_sync();
}

static inline void send_environment_data(void* buf, size_t size){
    swritef("/environment/data", buf, size, true);
    env_data_sync();
}
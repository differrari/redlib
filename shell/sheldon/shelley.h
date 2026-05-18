#pragma once

#include "syscalls/syscalls.h"

#define NARGS_I(_0,_1,_2,_3,_4,_5,_6,_7,N,...) N
#define EXPAND(x,...) x
#define NARGS(...) EXPAND(NARGS_I(_,##__VA_ARGS__,7,6,5,4,3,2,1,0))

#define SHELLEY_ARG(_name, _use_option, _position, _ignore_spaces, _option, _use_value, _optional) {\
    .name = {.data = (char*)(#_name), .length = sizeof(#_name)-1},\
    .indicator = {\
        .use_option = (_use_option),\
        .position = (_position),\
        .option = {.data = (char*)(_option), .length = sizeof(_option)-1},\
        .ignore_spaces = (_ignore_spaces),\
        .use_value = (_use_value),\
    },\
    .optional = (_optional),\
}

#define SHELLEY_ARG_POS_ALL(name, optional, position) SHELLEY_ARG(name, false, position, true, "", true, optional)
#define SHELLEY_ARG_POS(name, optional, position)  SHELLEY_ARG(name, false, position, false, "", true, optional)
#define SHELLEY_ARG_OPT(name, optional, option, value) SHELLEY_ARG(name, true, 0, false, option, value, optional)
#define SHELLEY_ARG_OPT_ALL(name, optional, option, value) SHELLEY_ARG(name, true, 0, true, option, value, optional)

#define SHELLEY_CMD(_name, execute, ...) static cmd_returns _name(shell_handle *handle, hash_map_t *arguments){ execute }\
cmd_def _name##_def = {\
    .name = {.data = (char*)(#_name), .length = sizeof(#_name)-1},\
    .entry_point = _name,\
    .argc = sizeof((cmd_arg[]){__VA_ARGS__}) / sizeof(cmd_arg),\
    .arguments = {__VA_ARGS__},\
}

#define SHELLEY_CMD_PRINT(_name, _func) \
SHELLEY_CMD(_name, {\
    print(_func());\
    return exit_return_success;\
})

#define SHELLEY_CMD_FWD(_name, _func) \
SHELLEY_CMD(_name, {\
    _func();\
    return exit_return_success;\
})

#define SHELLEY_CMD_FWD_1ARG(_name, _func, _arg) \
SHELLEY_CMD(_name, {\
    SHELLEY_GET_ARG(_arg);\
    _func(_arg);\
    return exit_return_success;\
}, SHELLEY_ARG_POS_ALL(_arg, false, 0))

#define SHELLEY_GET_ARG(argument) char* argument = hash_map_get_dictionary(arguments, #argument)
#define SHELLEY_GET_ARG_OR_DEFAULT(argument,default) char* argument = hash_map_get_dictionary(arguments, #argument); if (!argument) argument = default
#define SHELLEY_GET_ARG_PARSE(argument,type,default,parser) char* str_##argument = hash_map_get_dictionary(arguments, #argument); type argument = !str_##argument ? default : parser(str_##argument,strlen(str_##argument));
#pragma once

#define SHELLEY_ARG(_name, _use_option, _position, _ignore_spaces, _option, _use_value, _optional) ((cmd_arg){\
    .name = (SLICE_LIT(#_name)),\
    .indicator = {\
        .use_option = (_use_option),\
        .position = (_position),\
        .option = (_option),\
        .ignore_spaces = (_ignore_spaces),\
    },\
    .optional = (_optional),\
})

#define SHELLEY_ARG_POS_ALL(name, optional, position) SHELLEY_ARG(name, false, position, true, (string_slice){}, true, optional)
#define SHELLEY_ARG_POS(name, optional, position)  SHELLEY_ARG(name, false, position, false, (string_slice){}, true, optional)
#define SHELLEY_ARG_OPT(name, optional, option, value) SHELLEY_ARG(name, true, 0, false, option, value, optional)
#define SHELLEY_ARG_OPT_ALL(name, optional, position) SHELLEY_ARG(name, true, 0, true, option, value, optional)

#define SHELLEY_CMD(_name, _argc, _arguments, execute) static cmd_returns _name(shell_handle *handle, hash_map_t *arguments){ execute }\
cmd_def _name##_def = {\
    .name = SLICE_LIT(#_name),\
    .entry_point = _name,\
    .argc = _argc,\
    .arguments = _arguments,\
}

#define SHELLEY_GET_ARG(argument) char* argument = hash_map_get_dictionary(arguments, #argument)
#define SHELLEY_GET_ARG_PARSE(argument,type,default,parser) char* str_##argument = hash_map_get_dictionary(arguments, #argument); type argument = !str_##argument ? default : parser(str_##argument,strlen(str_##argument));
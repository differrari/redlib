#pragma once

typedef enum {
    pipe_default = 0,//Any writes to the source while the pipe is open get copied to the destination
    pipe_from_beginning = 1,//Read the source file when creating the pipe and copy its contents 
} pipe_options;
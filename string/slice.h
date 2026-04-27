#pragma once
#include "string.h"

typedef struct {
    char *data;
    size_t length;
} string_slice;

#define SLICE(str) ((string_slice){ .data = str, .length = sizeof(str)-1})

string_slice make_string_slice(const char* buf, size_t start, size_t length);
static inline string string_from_slice(string_slice slice){
    return string_from_literal_length(slice.data, slice.length);
}

static inline string_slice slice_from_string(string str){
    return (string_slice){.data = str.data,.length = str.length};
}

static inline string_slice slice_from_literal(const char* lit){
    return (string_slice){.data = (char*)lit, .length = strlen(lit)};
}

static inline bool slices_equal(string_slice sl1, string_slice sl2, bool case_insensitive){
    if (sl1.length != sl2.length) return false;
    for (size_t i = 0; i < sl1.length; i++)
        if (sl1.data[i] != sl2.data[i] && (!case_insensitive || tolower(sl1.data[i]) != tolower(sl2.data[i]))) return false;
    return true;
}

static inline bool slice_lit_match(string_slice sl, const char *lit, bool case_insensitive){
    return slices_equal(sl, make_string_slice(lit,0,strlen(lit)), case_insensitive);
}

static inline sizedptr slice_to_sizedptr(string_slice slice){
    return (sizedptr){.ptr = (uptr)slice.data,.size = slice.length};
}

typedef struct {
    char seek;
    char *str;
    size_t length;
    string_slice current;
    uptr pointer;
    bool allow_empty;
} string_splitter;

void string_split(const char *str, char seek, void (*perform)(string_slice slice));

static inline string_splitter make_string_splitter(const char *str, char seek, bool allow_empty){
    return (string_splitter){
        .seek = seek,
        .str = (char*)str,
        .length = strlen(str),
        .current = (string_slice){},
        .pointer = 0,
        .allow_empty = allow_empty
    };
}

bool string_splitter_advance(string_splitter *splitter);

bool string_quick_split(char *str, char seek, string_slice *lhs, string_slice *rhs);

static inline string_slice string_splitter_remaining(string_splitter *splitter){
    return (string_slice){.data = splitter->str + splitter->pointer, .length = splitter->length - splitter->pointer};
}
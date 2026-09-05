#pragma once

#include "types.h"
#include "string/slice.h"
#include "data/struct/stack.h"

/*
    Current syntax
    +: match 1 or more times
    ?: match 0 or 1 times (optional)
    *: match 0 or more times
    (: begin capture group
    ): end capture group
    [: begin character class
    ]: end character class
    n-m: within a character class, match any characters between n and m, inclusive
    ^: match opposite of next character, within a character class, match the opposite of all ranges
    .: match any character
   
    Currently only supports ascii characters.
    Does not support (not a comprehensive list, the list above is a comprehensive list of its syntax, anything not there is not supported):
        - {n,m}: matching an exact number of times
        - character classes using \ such as \w or \s
*/

typedef enum {
    regex_node_match,
    regex_node_start_group,
    regex_node_end_group,
    // TODO: Non-capture groups (?:)
    // TODO: Capture groups need to group things, not just capture
} regex_node_type;

typedef struct regex_node {
    char literal;
    char end;
    bool invert;
    bool any;
    regex_node_type type;
    int success;
    int fail;
} regex_node;

typedef struct {
    arr_stack_t *regex_stack;
} regex_handle;

#define MAX_CAPTURE_GROUPS 16

typedef struct {
    string_slice full_slice;
    bool found;
    range_t result_range;
    u64 capture_count;
    range_t capture_groups[MAX_CAPTURE_GROUPS];
} regex_result;

regex_handle init_regex(const char *pattern);
regex_handle init_manual_regex(regex_node nodes[], size_t count);

void regex_debug(regex_handle *handle);

regex_result regex_find_one(regex_handle *handle, string_slice str);
bool regex_find_many(regex_handle *handle, string_slice str, bool (*on_find)(regex_result));
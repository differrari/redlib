#define REGEX_IMPLEMENTATION
#ifdef REGEX_IMPLEMENTATION

#include "regex.h"
#include "syscalls/syscalls.h"
#include "utils/indent.h"
#include "memory/memory.h"

// #define REGEX_DEBUG
#ifdef REGEX_DEBUG
#define regex_print(...) print(__VA_ARGS__)
#else
#define regex_print(...)
#endif

regex_node* regex_get_node(regex_handle *handle, int index){
    if (index < 0 || (u64)index > stack_count(handle->regex_stack))
        return 0;
    return stack_get(handle->regex_stack, index);
}

regex_node* regex_new_node(regex_handle *handle){
    return regex_get_node(handle, stack_push(handle->regex_stack, &(regex_node){}));
}

regex_node* regex_clone_node(regex_handle *handle, regex_node *node){
    regex_node* new_node = regex_new_node(handle);
    memcpy(new_node, node, sizeof(regex_node));
    return new_node;
}

typedef enum { 
    regex_append_invalid, 
    regex_append_success = 1 << 1, 
    regex_append_failure = 1 << 2
} regex_state_append;

#define link_node(node) if (current_append_rule & regex_append_failure) previous_node->fail = node;\
if (current_append_rule & regex_append_success) previous_node->success = node;


regex_handle init_manual_regex(regex_node nodes[], size_t count){
    regex_handle handle = {};
    handle.regex_stack = stack_create(sizeof(regex_node), count);
    for (u64 i = 0; i < count; i++)
        stack_push(handle.regex_stack, &nodes[i]);
    return handle;
}

regex_handle init_regex_slice(string_slice pattern){
    bool ignore_next = false;

    regex_handle handle = {};
    handle.regex_stack = stack_create(sizeof(regex_node), 16);
    
    regex_node *previous_node = 0;
    regex_state_append current_append_rule = regex_append_success;
    bool should_invert = false;
    bool in_class = false;
    bool class_range_reuse = false;
    int backtrack_pointer = -1;
    for (u64 i = 0; i < pattern.length; i++){
        char next_char = pattern.data[i];
        if (!ignore_next){
            if (next_char == '\\'){
                ignore_next = true;
                continue;
            }
            if (next_char == '*'){
                previous_node->success = 0;
                current_append_rule = regex_append_failure;
                continue;
            }
            if (next_char == '+'){
                regex_node *new_node = regex_clone_node(&handle, previous_node);
                previous_node->success = 1;
                new_node->success = 0;
                previous_node = new_node;
                current_append_rule = regex_append_failure;
                continue;
            }
            if (next_char == '^'){
                should_invert = true;
                continue;
            }
            if (next_char == '?'){
                current_append_rule = regex_append_success | regex_append_failure;
                continue;
            }
            if (next_char == '(' || next_char == ')'){
                regex_node *node = regex_new_node(&handle);
                link_node(1);
                node->type = next_char == '(' ? regex_node_start_group : regex_node_end_group;
                current_append_rule = regex_append_success | regex_append_failure;
                previous_node = node;
                continue;
            }
            if (next_char == '-' && in_class){
                class_range_reuse = true;
                continue;
            }
            if (next_char == '['){
                if (in_class) continue;
                in_class = true;
                backtrack_pointer = stack_count(handle.regex_stack);
                if (pattern.length > i+1 && pattern.data[i+1] == '^'){
                    should_invert = true;
                    i++;
                }
                continue;
            }
            if (next_char == ']'){
                if (!in_class) continue;
                
                size_t count = stack_count(handle.regex_stack)-backtrack_pointer;
                for (u64 n = 0; n < count; n++){
                    regex_node *node = stack_get(handle.regex_stack, n);
                    node->success = count-n;
                    if (n == count-1) node->fail = 0;
                }

                in_class = false;
                should_invert = false;
                continue;
            }
        }
        regex_node *node = class_range_reuse ? previous_node : regex_new_node(&handle);
        if (class_range_reuse){
            node->end = next_char;
        } else {
            if (next_char == '.' && !ignore_next){
                node->any = true;
            } else 
                node->literal = next_char;
        }
        if (!node->any) node->invert = should_invert;
        if (!in_class) should_invert = false;
        if (previous_node){
            link_node(1);
        }
        if (!class_range_reuse) previous_node = node;
        class_range_reuse = false;
        // print("Expecting %c",next_char);
        ignore_next = false;
        current_append_rule = in_class ? regex_append_failure : regex_append_success;
    }
    
    return handle;
}

void regex_debug_node(regex_node *node){
    switch (node->type){
        case regex_node_start_group: 
            print("Start capture group"); break;
        case regex_node_end_group: 
            print("End capture group"); break;
        default: {
            char buf[10] = {};
            if (node->end){
                string_format_buf(buf,10,"%c-%c",node->literal,node->end);
            } else {
                buf[0] = node->literal;
                buf[1] = 0;
            }
            print("%s%s s: %i f: %i",node->invert ? "^" : "",buf, node->success,node->fail); 
            break;
        }
    }
}

void regex_debug(regex_handle *handle){
    if (!handle) {
        print("No handle");
        return;
    }
    if (!handle->regex_stack){
        print("No stack");
        return;
    }
    size_t count = stack_count(handle->regex_stack);
    for (u64 i = 0; i < count; i++)
        regex_debug_node(regex_get_node(handle, i));
}

regex_handle init_regex(const char *pattern){
    return init_regex_slice(slice_from_literal(pattern));
}

regex_result find_one_result = {};

bool regex_find_one_handler(regex_result result){
    find_one_result = result;
    return false;
}

regex_result regex_find_one(regex_handle *handle, string_slice str){
    find_one_result = (regex_result){};
    regex_find_many(handle, str, regex_find_one_handler);
    return find_one_result;
}

static inline bool regex_does_match(char c, regex_node *node){
    bool condition = node->end ? (c >= node->literal && c <= node->end) : (node->literal == c);
    return node->invert ^ condition;
}

bool regex_find_many(regex_handle *handle, string_slice str, bool (*on_find)(regex_result)){
    if (!handle || !handle->regex_stack || !str.length)        
        return false;
    int node_index = 0;
    regex_result result = {.full_slice = str};
    bool ever_found = false;
    result.found = false;
    bool is_capture_group = false;
    size_t count = stack_count(handle->regex_stack);
    for (u64 i = 0; i < str.length; i++){
        regex_node *current_node = regex_get_node(handle, node_index);
        if (!current_node) break;
        if (current_node->type != regex_node_match){
            regex_print("Node type is %i",current_node->type);
            switch (current_node->type){
                case regex_node_start_group:
                    if (is_capture_group){
                        print("[REGEX implementation error] nesting capture groups not allowed");
                        return false;
                    }
                    if (result.capture_count >= MAX_CAPTURE_GROUPS){
                        print("[REGEX implementation error] there is a maximum of %i capture groups",MAX_CAPTURE_GROUPS);
                        return false;
                    }
                    is_capture_group = true;
                    result.capture_count++;
                    result.capture_groups[result.capture_count].start = i;
                    node_index += current_node->success;
                    i--;
                    continue;
                case regex_node_end_group:
                    if (!is_capture_group){
                        print("[REGEX error] ending unknown capture group");
                        return false;
                    }
                    is_capture_group = false;
                    node_index += current_node->success;
                    i--;
                    continue;
                    break;
                default:
                    break;
            }
            continue;
        }
        
        char current_char = str.data[i];
        if (current_node->any) regex_print("[ANY]");
        else regex_print("%c vs %c",current_node->literal,current_char);
        
        if (current_node->any || regex_does_match(current_char, current_node)){
            regex_print("Succeded");
            if (is_capture_group){
                result.capture_groups[result.capture_count].size++;
            }
            if (current_node == regex_get_node(handle, 0)) result.result_range.start = i;
            if (current_node->success && node_index + current_node->success < (i64)count) node_index += current_node->success;
            else {
                ever_found = true;
                result.found = true;
                result.result_range.size = i-result.result_range.start;
                result.full_slice = str;
                if (!on_find(result)) break;
                result = (regex_result){};
                continue;
            }
        } else if (node_index != 0 || current_node->fail) {
            regex_print("Did not match");
            if (current_node->fail){
                i--;
                node_index += current_node->fail;
            } 
            else {
                regex_print("Found %c instead of expected %c",current_char, current_node->literal);
                node_index = 0;
                result = (regex_result){};
                continue;
            }
        }
    }
    return ever_found;
}

#endif
#define REGEX_IMPLEMENTATION
#ifdef REGEX_IMPLEMENTATION

#include "regex.h"
#include "syscalls/syscalls.h"
#include "memory/memory.h"
#include "data/format/scanner/scanner.h"

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

#define MAX_CAPTURE 8

static inline void close_group(Scanner *scanner, regex_result *result, int *capture_positions, int *capture_index){
    int index = *capture_index;
    int start = capture_positions[--index];
    *capture_index = index;
    int length = scanner->pos - start;
    result->capture_groups[result->capture_count++] = (range_t){.start = start, .size = length };
}

static inline bool handle_result(regex_result *result, Scanner *scanner, bool (*on_find)(regex_result)){
    regex_print("[REGEX debug] End of regex pattern");
    result->found = true;
    result->result_range.size = scanner->pos - result->result_range.start;
    char next = scan_peek(scanner);
    if (!on_find(*result) || !next){
        return true;
    }
    return false;
}

bool regex_find_many(regex_handle *handle, string_slice str, bool (*on_find)(regex_result)){
    if (!handle || !handle->regex_stack || !str.length)
        return false;
    regex_result result = {.full_slice = str};
    result.found = false;

    int node_index = 0;
    
    Scanner scanner = scanner_make(str.data, str.length);

    int capture_positions[MAX_CAPTURE] = {};
    int capture_index = 0;

    char current = 0;

    bool range_started = false;
    while (true){
        regex_node *node = stack_get(handle->regex_stack, node_index);
        if (!node){
            if (handle_result(&result, &scanner, on_find)){
                regex_print("[REGEX debug] Stop");
                return true;
            }
            regex_print("[REGEX debug] Continue");
            node_index = 0;
            capture_index = 0;
            range_started = false;
            result = (regex_result){ .full_slice = str };
            continue;
        }

        if (node->type == regex_node_start_group){
            regex_print("[REGEX debug] capture group start");
            if (capture_index + 1 >= MAX_CAPTURE){
                print("[REGEX error] too many nested captures");
                return false;
            }
            capture_positions[capture_index++] = scanner.pos;
            node_index += node->success ?: 1;
            continue;
        } else if (node->type == regex_node_end_group){
            regex_print("[REGEX debug] capture group end");
            close_group(&scanner, &result, capture_positions, &capture_index);
            node_index += node->success ?: 1;
            continue;
        }
        
        current = scan_next(&scanner);
        regex_print("[REGEX debug] Next character %c and index %i",current,node_index);
        // if (!current) continue;
        if (current && regex_does_match(current, node)){
            regex_print("[REGEX debug] success");
            if (!range_started){
                range_started = true;
                result.result_range.start = scanner.pos-1;
            }
            node_index += node->success;
        } else {
            regex_print("[REGEX debug] failure");
            regex_print("[REGEX debug] Backtrack because at %i %c didn't match %c",node_index,current, node->literal);
            if (!node->fail){
                if (range_started) scanner.pos--;
                if (!current) return false;
                capture_index = 0;
                node_index = 0;
                range_started = false;
                result = (regex_result){.full_slice = str};
            } else {
                if (current) scanner.pos--;
                node_index += node->fail;
            }
        }
    }
    
    return false;
}

#endif
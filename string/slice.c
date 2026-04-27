#include "slice.h"
#include "math/math.h"

string_slice make_string_slice(const char* buf, size_t start, size_t length){
    if (strlen(buf) <= start) return (string_slice){};
    length = min(strlen(buf+start),length);
    return (string_slice){.data = (char*)buf + start, .length = length };
}

void string_split(const char *str, char seek, void (*perform)(string_slice slice)){
    if (!str) return;
    uintptr_t cursor = 0;
    char *point = (char*)str;
    do {
        char *new_point = (char*)seek_to(point, seek);
        if (new_point == point) break;
        size_t size = new_point-point-(*new_point || *(new_point-1) == '/' ? 1 : 0);
        perform((string_slice){.data = (char*)str + cursor, size});
        point = new_point;
        cursor += size+1;
        if (*point == 0) break;
    } while(point);
}

bool string_splitter_advance(string_splitter *splitter){
    if (!splitter->str || !splitter->seek) return false;
    if (splitter->pointer >= splitter->length) return false;
    char *current = splitter->str + splitter->pointer;
    if (*current == 0) return false;
    char *new_point = (char*)seek_to(current, splitter->seek);
    if (new_point == current) return false;
    size_t size = new_point-current-(*new_point || *(new_point-1) == '/' ? 1 : 0);
    splitter->current = (string_slice){.data = (char*)splitter->str + splitter->pointer, size};
    splitter->pointer += size+1;
    if (!size && !splitter->allow_empty) return string_splitter_advance(splitter);
    return true;
}

bool string_quick_split(char *str, char seek, string_slice *lhs, string_slice *rhs){
    if (!lhs || !rhs) return false;
    size_t full_size = strlen(str);
    if (!full_size) return false;
    char *new_point = (char*)seek_to(str, seek);
    if (new_point == str) return false;
    size_t size = new_point-str-(*new_point || *(new_point-1) == '/' ? 1 : 0);
    *lhs = (string_slice){.data = str, .length = size};
    *rhs = (string_slice){.data = str + size, .length = full_size-size};
    return true;
}

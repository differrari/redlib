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
        size_t size = new_point-point-(*new_point ? 1 : 0);
        perform((string_slice){.data = (char*)str + cursor, size});
        point = new_point;
        cursor += size+1;
        if (*point == 0) break;
    } while(point);
}
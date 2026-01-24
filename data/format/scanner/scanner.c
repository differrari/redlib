#include "scanner.h"

bool scan_eof(Scanner *s) {
    return s->pos >= s->len;
}

char scan_peek(Scanner *s) {
    if (s->pos >= s->len) return 0;
    return s->buf[s->pos];
}

char scan_next(Scanner *s) {
    if (s->pos >= s->len) return 0;
    return s->buf[s->pos++];
}

bool scan_match(Scanner *s, char c) {
    if (scan_eof(s)) return false;
    if (s->buf[s->pos] != c) return false;
    s->pos++;
    return true;
}

bool scan_match_string(Scanner *s, const char *str) {
    uint32_t i = 0;
    while (str[i]) {
        if (s->pos + i >= s->len) return false;
        if (s->buf[s->pos + i] != str[i]) return false;
        i++;
    }
    s->pos += i;
    return true;
}

void scan_skip_ws(Scanner *s, bool skip_nl) {
    while (!scan_eof(s)) {
        char c = s->buf[s->pos];
        if (c==' '||(skip_nl && c=='\n')||c=='\t'||c=='\r') s->pos++;
        else break;
    }
}

#include "syscalls/syscalls.h"

string_slice scan_to(Scanner *s, char seek){
    uint32_t initial = s->pos;
    while (!scan_eof(s)) {
        char c = s->buf[s->pos++];
        if (c == seek) break;
    }
    if (s->pos == s->len && initial == s->pos) return (string_slice){.data = (char*)s->buf + initial, .length = 0 };
    return (string_slice){.data = (char*)s->buf + initial, .length = s->pos-initial };
}
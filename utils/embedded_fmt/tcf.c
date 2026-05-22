#include "embedded_fmt.h"
#include "alloc/allocate.h"
#include "string/slice.h"
#include "string/string.h"
#include "syscalls/syscalls.h"
#include "memory/memory.h"

#define TCF_MAX_BUF 16

typedef struct {
    int in_sequence;
    enum { tcf_state_none, tcf_state_cmd, tcf_state_arg } in_cmd;
    char buf[TCF_MAX_BUF];
    u8 buf_cursor;
    embedded_fmt_feature current_feature;
} tcf_state;

bool tcf_parse_type(embedded_fmt *formatter, tcf_state *state){
    printl(state->buf);
    if (state->in_cmd == tcf_state_none) return false;
    if (state->in_cmd == tcf_state_cmd){
        string_slice current_cmd = {state->buf, state->buf_cursor};
        if (slice_lit_match(current_cmd, "color", true)){
            state->current_feature = embeddef_fmt_color;
        }
        if (slice_lit_match(current_cmd, "wipe", true)){
            state->current_feature = embeddef_fmt_screen;
            formatter->wipe = true;
        }
    }
    if (state->in_cmd == tcf_state_arg){
        switch (state->current_feature) {
        case embeddef_fmt_color: {
            u64 hex = parse_hex_u64(state->buf, state->buf_cursor);
            hex |= (0xFF << 24);
            formatter->current_text_color = hex;
        } break;
        default:
          break;
        }
    }
    memset(state->buf, 0, state->buf_cursor);
    state->buf_cursor = 0;
    return true;
}

void tcf_reset_format(embedded_fmt *formatter){
    formatter->current_text_color = formatter->default_text_color;
}

bool embedded_fmt_parse(embedded_fmt *formatter, char c){
    if (formatter->state_type != DATA_SIGNATURE("TCF")){
        if (formatter->state_type) return false;
        formatter->state_type = DATA_SIGNATURE("TCF");
        formatter->state = zalloc(sizeof(tcf_state));
    }

    tcf_state *state = formatter->state;
    
    if (c == '{'){
        if (state->in_cmd != tcf_state_none) return false;
        state->in_sequence++;
        state->in_cmd = tcf_state_cmd;
        return true;
    } else if (c == '}'){
        tcf_parse_type(formatter, state);
        state->in_sequence--;
        state->in_cmd = tcf_state_none;
        tcf_reset_format(formatter);
        if (state->in_sequence < 0){
            return false;
        }
        return true;
    } else if (state->in_cmd != tcf_state_none){
        if (c == ':'){
            if (state->in_cmd != tcf_state_cmd) return false;
            tcf_parse_type(formatter, state);
            state->in_cmd = tcf_state_arg;
        } else if (c == ','){
            if (state->in_cmd != tcf_state_arg) return false;
            tcf_parse_type(formatter, state);
        } else if (c == ' '){
            tcf_parse_type(formatter, state);
            state->in_cmd = tcf_state_none;
        } else if (state->buf_cursor < TCF_MAX_BUF){
            state->buf[state->buf_cursor++] = c;
        }
        return true;
    }
    if (formatter->current_text_color == 0xFF666000){
        char bf[10] = {};
        bf[0] = c;
        printl(bf);
    }
    return false;
}
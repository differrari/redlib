#include "tcf.h"
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
    int arg_num;
} tcf_state;

void init_tcf(embedded_fmt *formatter){
    if (formatter->state || formatter->state_type) return;
    formatter->state_type = DATA_SIGNATURE("TCF");
    formatter->state = zalloc(sizeof(tcf_state));
}

bool tcf_parse_type(embedded_fmt *formatter, tcf_state *state){
    if (state->in_cmd == tcf_state_none) return false;
    if (state->in_cmd == tcf_state_cmd){
        string_slice current_cmd = {state->buf, state->buf_cursor};
        if (slice_lit_match(current_cmd, "color", true) || slice_lit_match(current_cmd, "fg", true)){
            state->current_feature = embeddef_fmt_color;
        }
        if (slice_lit_match(current_cmd, "background", true) || slice_lit_match(current_cmd, "bg", true)){
            state->current_feature = embeddef_fmt_bg;
        }
        if (slice_lit_match(current_cmd, "wipe", true)){
            state->current_feature = embeddef_fmt_screen;
            formatter->wipe = true;
        }
        if (slice_lit_match(current_cmd, "cursor", true)){
            state->current_feature = embeddef_fmt_cursor_abs;
        }
        if (slice_lit_match(current_cmd, "movecursor", true) || slice_lit_match(current_cmd, "movecur", true)){
            state->current_feature = embeddef_fmt_cursor_rel;
        }
    }
    if (state->in_cmd == tcf_state_arg){
        switch (state->current_feature) {
        case embeddef_fmt_color:
        case embeddef_fmt_bg: 
        {
            u64 hex = parse_hex_u64(state->buf, state->buf_cursor);
            hex |= (0xFF << 24);
            if (state->current_feature == embeddef_fmt_color) 
                formatter->current_text_color = hex;
            else
                formatter->current_bg_color = hex;
        } break;
        case embeddef_fmt_cursor_abs:
        case embeddef_fmt_cursor_rel:
        {
            i64 val = parse_int64(state->buf, state->buf_cursor);
            if (state->arg_num == 0){
                if (state->current_feature == embeddef_fmt_cursor_rel) 
                    formatter->cursor_x += val;
                else
                    formatter->cursor_x = val;
                // if (formatter->cursor_x > max_x) formatter->cursor_x = 0;
            } else {
                if (state->current_feature == embeddef_fmt_cursor_rel) 
                    formatter->cursor_y += val;
                else
                    formatter->cursor_y = val;
                // if (formatter->cursor_y > max_y) formatter->cursor_y = 0;
            }
        }
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
    formatter->current_bg_color = formatter->default_bg_color;
}

bool embedded_fmt_parse(embedded_fmt *formatter, char c){
    if (!formatter->state) return false;

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
            state->arg_num = 0;
        } else if (c == ','){
            if (state->in_cmd != tcf_state_arg) return false;
            tcf_parse_type(formatter, state);
            state->arg_num++;
        } else if (c == ' '){
            tcf_parse_type(formatter, state);
            state->in_cmd = tcf_state_none;
            state->arg_num = 0;
        } else if (state->buf_cursor < TCF_MAX_BUF){
            state->buf[state->buf_cursor++] = c;
        }
        return true;
    }
    return false;
}
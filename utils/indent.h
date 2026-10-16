#pragma once

extern char *_indent;
#define MAX_DEPTH 64
#define indent_by(depth) (_indent + (MAX_DEPTH-(depth >= MAX_DEPTH ? 1 : depth)))
#pragma once

#define REGEX_ADVANCE 1
#define REGEX_STAY 0
#define REGEX_FAIL 0

#define REGEX_MATCH_FULL(c,e,i,_success,_fail) ((regex_node){ .literal = c, .end = e, .invert = i, .type = regex_node_match, .success = _success, .fail = _fail })
#define REGEX_MATCH(c,_success,_fail) REGEX_MATCH_FULL(c, 0, false, _success,_fail)
#define REGEX_NOT(c,_success,_fail) REGEX_MATCH_FULL(c, 0, true, _success,_fail)
#define REGEX_RANGE(st,end,_success,_fail) REGEX_MATCH_FULL(st,end, false ,_success,_fail)
#define REGEX_CAPTURE(...) \
(regex_node){ .literal = 0,    .invert = false, .type = regex_node_start_group, .success = 1 },\
__VA_ARGS__,\
(regex_node){ .literal = 0,    .invert = false, .type = regex_node_end_group, .success = 1 }

#define REGEX_LITERAL(c) REGEX_MATCH(c, REGEX_ADVANCE, REGEX_FAIL)
#define REGEX_NOT_LITERAL(c) REGEX_NOT(c, REGEX_ADVANCE, REGEX_FAIL)

#define REGEX_ONE_OR_MORE(c) REGEX_MATCH(c, REGEX_ADVANCE, REGEX_FAIL) REGEX_MATCH(c, REGEX_STAY, REGEX_ADVANCE)
#define REGEX_ZERO_OR_MORE(c) REGEX_MATCH(c, REGEX_STAY, REGEX_ADVANCE)

#define REGEX_ONE_OR_MORE_NOT(c) REGEX_NOT(c, REGEX_ADVANCE, REGEX_FAIL) REGEX_NOT(c, REGEX_STAY, REGEX_ADVANCE)
#define REGEX_ZERO_OR_MORE_NOT(c) REGEX_NOT(c, REGEX_STAY, REGEX_ADVANCE)
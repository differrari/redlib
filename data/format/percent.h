#pragma once

#include "types.h"
#include "string/string.h"

#ifdef __cplusplus
extern "C" {
#endif

bool percent_decode(sizedptr in, string *out);

#ifdef __cplusplus
}
#endif

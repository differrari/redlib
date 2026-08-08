#pragma once

#include "types.h"
#include "std/string.h"

typedef struct {
    bool valid;
    string name;
    string author;
    string version;
    string id;
} package_info;

#ifdef __cplusplus
extern "C" {
#endif
package_info parse_package_info(char *info);
void package_info_dispose(package_info* info);
#ifdef __cplusplus
}
#endif
#include "package_info.h"
#include "data/format/toml.h"
#include "data/format/csv.h"
#include "syscalls/syscalls.h"

void handle_package_entitlements(string_slice slice, void *ctx){
    if (slice.length && slice.data[0] == '"'){
        slice.length--;
        slice.data++;
    }
    if (slice.length && slice.data[slice.length-1] == '"'){
        slice.length--;
    }
    package_info *pkg_info = (package_info*)ctx;
    if (!pkg_info->entitlements) 
        pkg_info->entitlements = chunk_array_create(sizeof(string), 16);
    string s = string_from_literal_length(slice.data, slice.length);
    chunk_array_push(pkg_info->entitlements, &s);
    print("Added entitlement %S",s);
}

static inline void handle_package_kvp(string_slice key, string_slice value, void* ctx){
    package_info *pkg_info = (package_info*)ctx;
    if ((size_t)strstart_case("app_name", key.data,true) == key.length) 
        pkg_info->name = string_from_literal_length(value.data, value.length);
    if ((size_t)strstart_case("app_author", key.data,true) == key.length)
        pkg_info->author = string_from_literal_length(value.data, value.length);
    if ((size_t)strstart_case("app_version", key.data,true) == key.length)
        pkg_info->version = string_from_literal_length(value.data, value.length);
    if ((size_t)strstart_case("app_bundle_id", key.data,true) == key.length)
        pkg_info->id = string_from_literal_length(value.data, value.length);
    if ((size_t)strstart_case("app_entitlements", key.data,true) == key.length){
        read_csv(value, handle_package_entitlements, pkg_info);
    }
}

package_info parse_package_info(char *info){
    package_info pkg_info = {};
    if (info) read_toml(info, handle_package_kvp, (void*)&pkg_info);
    return pkg_info;
}

void package_info_dispose(package_info* info){
    string_free(info->name);
    string_free(info->author);
    string_free(info->version);
    string_free(info->id);
}
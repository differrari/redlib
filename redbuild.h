#pragma once

#include "types.h"
#include "string/string.h"
#include "string/slice.h"
#include "syscalls/syscalls.h"
#include "files/helpers.h"
#include "files/buffer.h"
#include "alloc/allocate.h"
#include "data/struct/linked_list.h"

typedef enum {
    target_none,
    target_redacted,
    target_linux,
    target_mac,
    target_windows,
    target_native,
} target;
//TODO: target must be choosable dynamically
//TODO: can we make it rebuild itself before compiling?

typedef enum { dep_local, dep_framework, dep_system } dependency_type;

typedef struct {
    dependency_type type;
    char *link;
    char *include;
    char *build;
    char *version;
    bool use_make;
} dependency_t; 

typedef enum { package_red, package_bin, package_lib } package_type;

target global_target;

typedef struct redb_ctx {
    linked_list_t *compile_list;
    linked_list_t *preproc_flags_list;
    linked_list_t *comp_flags_list;
    linked_list_t *link_flags_list_f;
    linked_list_t *link_flags_list_b;
    linked_list_t *includes;
    linked_list_t *link_libs;
    linked_list_t *ignore_list;
    linked_list_t *out_files;
    
    target selected_target;
    
    char *output_name;
    const char *homedir;
    string output;
    
    char *chosen_compiler;
    
    target compilation_target;
    package_type pkg_type;
    
    buffer buf;
    char *extension;
    bool debug_syms;
} redb_ctx;

static buffer ccbuf;
static redb_ctx *ctx;

#define REDBUILD_DEBUG
#ifdef REDBUILD_DEBUG
#define redbuild_debug(fmt, ...) print(fmt, ##__VA_ARGS__)
#else 
#define redbuild_debug(fmt, ...)
#endif

#if __linux
#define native_target target_linux
#elif _WIN32
#define native_target target_windows
#elif (__APPLE__)
#define native_target target_mac
#else 
#define native_target target_redacted
#endif

target parse_target(char* name){
    if (strcmp(name,"native") == 0) return target_native;
    if (strcmp(name,"linux") == 0) return target_linux;
    if (strcmp(name,"mac") == 0) return target_mac;
    if (strcmp(name,"windows") == 0) return target_windows;
    if (strcmp(name,"redacted") == 0) return target_redacted;
    return target_none;
}

literal target_to_string(target t){
    switch (t) {
        case target_none: return "none";
        case target_redacted: return "redacted";
        case target_linux: return "linux";
        case target_mac: return "mac";
        case target_windows: return "windows";
        case target_native: return "native";
      break;
    }
    return "none";
}

void set_global_target(target t);

void parse_arguments(int argc, char* argv[]){
    enum { arg_none, arg_target };
    int next_argument = 0;
    for (int i = 0; i < argc; i++){
        if (strlen(argv[i]) && *argv[i] == '-'){
            if (strcmp(argv[i],"-t") == 0)
                next_argument = arg_target;
            else 
                next_argument = arg_none;
        } else {
            switch (next_argument) {
                case arg_target: set_global_target(parse_target(argv[i])); break;
                default: break;
            }
            next_argument = arg_none;
        }
    }
}

void free_strings(void *data){
    string *s = (string*)data;
    string_free(*s);
}

void free_deps(void *data){
    dependency_t *dep = (dependency_t*)data;
    release(dep);
}

void push_lit(linked_list_t *list, const char* lit){
    string *s = zalloc(sizeof(string));
    *s = string_from_literal(lit);
    linked_list_push(list, s);
}

void add_dependency(dependency_type type, char *include, char *link, char* build, bool use_make);
void add_local_dependency(char *include, char *link, char* build, bool use_make);
void add_system_lib(char *name);
void add_system_framework(char *name);
void include_self();
void add_precomp_flag(char *name);
void add_compilation_flag(char *name);
void add_linker_flag(char *name, bool back);

void cross_mod(){
    redbuild_debug("Native platform setup");
    add_system_lib("c");
    add_system_lib("m");
    add_local_dependency("~/redlib", "~/redlib/clibshared.a", "~/redlib", true);
    add_system_lib("glfw");
    add_precomp_flag("CROSS");
    redbuild_debug("Common platform setup done");
    if (ctx->compilation_target == target_none || ctx->compilation_target == target_native) ctx->compilation_target = native_target;
    switch (ctx->compilation_target) {
        case target_linux: add_linker_flag("-Wl,--start-group", false); add_linker_flag("-Wl,--end-group", true);break;
        case target_windows: add_linker_flag("-fuse-ld=lld", false); break;
        case target_mac: {
            add_linker_flag("-L/opt/homebrew/lib", false);
            add_system_framework("Cocoa");
            add_system_framework("IOKit");
            add_system_framework("CoreVideo");
            add_system_framework("CoreFoundation");
            add_dependency(dep_local, "/opt/homebrew/include", 0, 0, 0);
        } break;
        default: break;
    }
    if (ctx->compilation_target == target_mac){
        add_system_framework("OpenGL");
    } else {
        add_system_lib("GL");
    }
    redbuild_debug("Platform specific setup done. Selecting compiler");
    ctx->chosen_compiler = "";
    redbuild_debug("Compiler selected");
    redbuild_debug("Compiler %s",ctx->chosen_compiler);
}

void red_mod(){
    ctx->chosen_compiler = "aarch64-none-elf-";
    add_local_dependency("~/redlib", "~/redlib/libshared.a", "~/os/", true);
    add_linker_flag("-Wl,-emain",false);
    add_linker_flag("-ffreestanding", false);
    add_linker_flag("-nostdlib", false);
}

void common(){
    redbuild_debug("Getting home dir");
    ctx->homedir = gethomedir();
    redbuild_debug("Home dir %s",ctx->homedir);
    
    include_self();
    
    add_compilation_flag("no-format-invalid-specifier");
    add_compilation_flag("no-format-extra-args");
}

void set_global_target(target t){
    if (t == target_native)
        t = native_target;
    global_target = t;
}

void set_target(target t, bool overwrite_global){
    if (!overwrite_global && global_target != target_none) ctx->compilation_target = global_target;
    if (t == target_native)
        ctx->compilation_target = native_target;
    else 
        ctx->compilation_target = t;
    redbuild_debug("Target type %i",ctx->compilation_target);
}

void set_name(const char *out_name){
    ctx->output_name = (char*)out_name;
}

void prepare_output(){
    if (!ctx->output_name) {
        print("No output name specified");
        return;
    }
    char *cwd = get_current_dir();
    switch (ctx->pkg_type) {
        case package_red: {
            //TODO: %s feel dangerous here
            string d = string_format("mkdir -p %s/%s.red",cwd,ctx->output_name);
            printf("Making %s/%s.red",cwd,ctx->output_name);
            system(d.data);
            //TODO: create copy recursive functions for copying directories (ffs)
            string cmd1 = string_format("cp -rf package.info %s.red/package.info",ctx->output_name);
            string cmd2 = string_format("cp -rf resources %s.red/resources",ctx->output_name);
            printf("%s",cmd1.data);
            system(cmd1.data);
            printf("%s",cmd2.data);
            system(cmd2.data);
            string_free(d);
            string_free(cmd2);
            string_free(cmd1);
            ctx->output = string_format("%s.red/%s.elf",ctx->output_name, ctx->output_name);
        } break;
        case package_bin:
            ctx->output = string_format("%s",ctx->output_name);
        break;
        case package_lib:
            ctx->output = string_format("%s.a",ctx->output_name);
        break;
    }
    redbuild_debug("Output environment ready");
}

void set_package_type(package_type type){
    ctx->pkg_type = type;
    redbuild_debug("Package type %i",ctx->pkg_type);
}

void destroy_module(redb_ctx *old_ctx){
    if (old_ctx->compile_list){
        if (old_ctx->compile_list->length) linked_list_for_each(old_ctx->compile_list, free_strings);
        linked_list_destroy(old_ctx->compile_list);
    }
    
    if (old_ctx->preproc_flags_list){
        if (old_ctx->preproc_flags_list->length) linked_list_for_each(old_ctx->preproc_flags_list, free_strings);
        linked_list_destroy(old_ctx->preproc_flags_list);
    }
    
    if (old_ctx->includes){
        if (old_ctx->includes->length) linked_list_for_each(old_ctx->includes, free_strings);
        linked_list_destroy(old_ctx->includes);
    }
    
    if (old_ctx->link_libs){
        if (old_ctx->link_libs->length) linked_list_for_each(old_ctx->link_libs, free_strings);
        linked_list_destroy(old_ctx->link_libs);
    }
    
    if (old_ctx->comp_flags_list){
        if (old_ctx->comp_flags_list->length) linked_list_for_each(old_ctx->comp_flags_list, free_strings);
        linked_list_destroy(old_ctx->comp_flags_list);
    }
    
    if (old_ctx->link_flags_list_f){
        if (old_ctx->link_flags_list_f->length) linked_list_for_each(old_ctx->link_flags_list_f, free_strings);
        linked_list_destroy(old_ctx->link_flags_list_f);
    }
    
    if (old_ctx->link_flags_list_b){
        if (old_ctx->link_flags_list_b->length) linked_list_for_each(old_ctx->link_flags_list_b, free_strings);
        linked_list_destroy(old_ctx->link_flags_list_b);
    }
    
    if (old_ctx->ignore_list){
        if (old_ctx->ignore_list->length) linked_list_for_each(old_ctx->ignore_list, free_strings);
        linked_list_destroy(old_ctx->ignore_list);
    }
    
    if (old_ctx->out_files){
        if (old_ctx->out_files->length) linked_list_for_each(old_ctx->out_files, free_strings);
        linked_list_destroy(old_ctx->out_files);
    }
    
    redbuild_debug("Finished cleanup");
}

void new_module(const char *name){
    printf("Compiling target %s",name);
    ctx = (redb_ctx*)zalloc(sizeof(redb_ctx));
    
    ctx->compile_list = linked_list_create();
    ctx->preproc_flags_list = linked_list_create();
    ctx->includes = linked_list_create();
    ctx->link_libs = linked_list_create();
    ctx->comp_flags_list = linked_list_create();
    ctx->link_flags_list_f = linked_list_create();
    ctx->link_flags_list_b = linked_list_create();
    ctx->ignore_list = linked_list_create();
    ctx->out_files = linked_list_create();
    ctx->debug_syms = false;
    ctx->selected_target = global_target;
}

bool source(const char *name){
    redbuild_debug("Adding %s",name);
    if (!ctx->compile_list || !ctx->out_files){ printf("Error: new_module not called"); return false; }
    push_lit(ctx->compile_list, name);
    
    string o = string_format("%v.o", make_string_slice(name,0,strlen(name)-2));
    push_lit(ctx->out_files, o.data);
    return false;
}

void add_dependency(dependency_type type, char *include, char *link, char* build, bool use_make){
    if (!ctx->homedir) ctx->homedir = gethomedir();
    if (include && strlen(include)){
        string s = string_replace_character(include, '~', (char*)ctx->homedir);
        string sf = string_format("-I%s ",s.data);
        push_lit(ctx->includes, sf.data);
        string_free(s);
        string_free(sf);
    } 
    if (link && strlen(link)){
        string l = {};
        string s = string_replace_character(link, '~', (char*)ctx->homedir);
        switch (type) {
            case dep_local: l = string_format(" %s", s.data); break;
            case dep_system: l = string_format(" -l%s",s.data); break;
            case dep_framework: l = string_format(" -framework %s",s.data); break;
        }
        if (l.data){
            push_lit(ctx->link_libs, l.data);
            string_free(l);
        }
        string_free(s);
    } 
}

void add_local_dependency(char *include, char *link, char* build, bool use_make){
    add_dependency(dep_local, include, link, build, use_make);
}

void add_system_lib(char *name){
    add_dependency(dep_system, "", name, "", false);
}

void add_system_framework(char *name){
    add_dependency(dep_framework, "", name, "", false);
}

void include_self(){
    add_dependency(dep_local, ".", "", "", false);   
}

void add_precomp_flag(char *name){
    push_lit(ctx->preproc_flags_list, name);
}

void add_compilation_flag(char *name){
    push_lit(ctx->comp_flags_list, name);
}

void add_linker_flag(char *name, bool back){
    push_lit(back ? ctx->link_flags_list_b : ctx->link_flags_list_f, name);
}

void list_strings(void *data){
    string *s = (string*)data;
    buffer_write(&ctx->buf, s->data);
    buffer_write_space(&ctx->buf);
}

void process_preproc_flags(void *data){
    string *s = (string*)data;
    buffer_write(&ctx->buf, "-D%s",s->data);
    buffer_write_space(&ctx->buf);
}

void process_comp_flags(void *data){
    string *s = (string*)data;
    buffer_write(&ctx->buf, "-W%s",s->data);
    buffer_write_space(&ctx->buf);
}

void prepare_command(char* source, char* out){
    redbuild_debug("Beginning compilation process");
    if (ctx->buf.buffer) buffer_destroy(&ctx->buf);
    ctx->buf = buffer_create(1024, buffer_can_grow);
    buffer_write(&ctx->buf, "%sgcc", ctx->chosen_compiler);
    buffer_write_space(&ctx->buf);
    
    linked_list_for_each(ctx->preproc_flags_list, process_preproc_flags);
    
    if (ctx->debug_syms){
        buffer_write_const(&ctx->buf," -g -fsanitize=address ");
    }
    
    linked_list_for_each(ctx->link_flags_list_f, list_strings);
    linked_list_for_each(ctx->includes, list_strings);
    
    if (source && out){
        buffer_write(&ctx->buf,"-c %s", source);
        buffer_write_space(&ctx->buf);
    } else {
        linked_list_for_each(ctx->compile_list, list_strings);
    }
    
    linked_list_for_each(ctx->link_libs, list_strings);
    linked_list_for_each(ctx->comp_flags_list, process_comp_flags);
    linked_list_for_each(ctx->link_flags_list_b, list_strings);
    
    if (source && out){
        buffer_write(&ctx->buf, "-o %s",out);
    } else {
        buffer_write(&ctx->buf, "-o %S",ctx->output);
    }
    redbuild_debug("Final compilation command:");
    printl(ctx->buf.buffer);
}

bool gen_compile_commands(const char *file);

void prepare_archive(){
    if (ctx->buf.buffer) buffer_destroy(&ctx->buf);
    ctx->buf = buffer_create(1024, buffer_can_grow);
    
    buffer_write(&ctx->buf, "%sar rcs ",ctx->chosen_compiler);
    
    buffer_write(&ctx->buf, "%S",ctx->output);
    
    gen_compile_commands(ctx->output.data);
    
    buffer_write_space(&ctx->buf);
    
    linked_list_for_each(ctx->out_files, list_strings);
    
    redbuild_debug("Final archive command:");
    redbuild_debug(ctx->buf.buffer);
}

bool compile(){
    redbuild_debug("Setting up output...");
    prepare_output();
    redbuild_debug("Target set. Common setup...");
    common();
    redbuild_debug("Common setup done. Platform-specific setup...");
    
    if (global_target != target_none) ctx->compilation_target = global_target;
    
    if (ctx->compilation_target == target_redacted) red_mod();
    else cross_mod();
    
    redbuild_debug("Platform-specific setup done.");
    
    if (ctx->pkg_type == package_lib){
        size_t file_count = linked_list_count(ctx->compile_list);
        for (size_t i = 0; i < file_count; i++){
            linked_list_node_t *srcn = linked_list_get(ctx->compile_list, i);
            linked_list_node_t *dstn = linked_list_get(ctx->out_files, i);
            if (!srcn || !dstn){
                print("Missing file");
                return false;
            }
            string* src = srcn->data;
            string* dst = dstn->data;
            if (!src || !dst){
                print("Missing file");
                return false;
            }
            prepare_command(src->data,dst->data);
            if (system(ctx->buf.buffer) != 0) return false;
            gen_compile_commands(src->data);
        }
        prepare_archive();
        return system(ctx->buf.buffer) == 0;
    } else {
        prepare_command(0,0);
        print("Compiling");
        return system(ctx->buf.buffer) == 0;
    }
}

void emit_argument(string_slice slice){
    if (!slice.length) return;
    buffer_write(&ccbuf,"\t\"%v\"",slice);
    buffer_write(&ccbuf,",\n");
}

bool gen_compile_commands(const char *file){
    if (!ccbuf.buffer){
        ccbuf = buffer_create(ctx->buf.buffer_size + 1024, buffer_can_grow);
        buffer_write(&ccbuf,"[");
    } else buffer_write(&ccbuf,",");
    
    //TODO: use code formatter
    buffer_write(&ccbuf,"{\n\"arguments\": [\n");
    string_split(ctx->buf.buffer,' ',emit_argument);
    ccbuf.buffer_size -= 2;//TODO: one of my stupidest hacks right here
    ccbuf.cursor -= 2;
    buffer_write(&ccbuf,"\n],\n\"directory\": \"");
    buffer_write(&ccbuf,get_current_dir());
    buffer_write(&ccbuf,"\",\n\"file\": \"");
    buffer_write(&ccbuf,get_current_dir());
    buffer_write(&ccbuf,"/%s\",\n\"output\": \"",file ? file : "main.c");
    buffer_write(&ccbuf,get_current_dir());
    buffer_write(&ccbuf,"/%s\"}",ctx->output);
    return true;
}

bool emit_compile_commands(){
    buffer_write(&ccbuf,"]");
    write_full_file("compile_commands.json",ccbuf.buffer,ccbuf.buffer_size);
    return true;
}

bool make_run(const char *directory, const char *command){
    buffer b = buffer_create(256, buffer_can_grow);
    buffer_write(&b, "make -C %s %s",directory, command);
    redbuild_debug("Final make command %s",b.buffer);
    return system(b.buffer) == 0;
}

void install(const char *location){
    string s = string_format("cp -r %s%s %s", ctx->output_name,ctx->pkg_type == package_red ? ".red" : "", location);
    system(s.data);
    string_free(s);
}

int run(){
    if (global_target == target_redacted || ctx->compilation_target == target_redacted){
        install("~/os_repo/applications");
        make_run("~/os","run");
        return 0;
    }
    string s = string_format("./%s",ctx->output);
    int result = system(s.data);
    string_free(s);
    return result;
}

bool cred_compile(){
    buffer b = buffer_create(1024, buffer_can_grow);
    buffer_write_const(&b, "cred ");
    linked_list_for_each(ctx->compile_list, list_strings);
    buffer_write(&b, "-o %s",ctx->output_name);
    redbuild_debug("Final compilation command:");
    printl(b.buffer);
    redbuild_debug("Generating C code");
    int ret = system(b.buffer) == 0;
    buffer_destroy(&b);
    return ret;
}

bool quick_cred(const char *input_file, const char *output_file){
    buffer b = buffer_create(1024, buffer_can_grow);
    buffer_write(&b, "cred %s -o %s",input_file,output_file);
    redbuild_debug("Final compilation command:");
    printl(b.buffer);
    redbuild_debug("Generating C code");
    int ret = system(b.buffer) == 0;
    buffer_destroy(&b);
    return ret;
}

int comp_str(void *a, void *b){
    return strcmp(((string*)a)->data,(char*)b);
}

void handle_files(const char *directory, const char *name){
    if (strend(name, ctx->extension)) return;
    if (linked_list_find(ctx->ignore_list, (char*)name, comp_str)) {
        redbuild_debug("Ignoring %s",name);
        return;
    }
    string n = string_format("%s/%s",directory,name);
    push_lit(ctx->compile_list, n.data);
    
    string o = string_format("%s/%v.o", directory, make_string_slice(name,0,strlen(name)-2));
    push_lit(ctx->out_files, o.data);
}

void find_files(char *ext){
    
    redbuild_debug("Adding all non-ignored files with %s extension",ext);
    
    char *cwd = get_current_dir();
    if (!cwd) { printf("No path"); return; }
    
    traverse_directory(cwd, true, handle_files);
}

bool ignore_source(const char *name){
    redbuild_debug("Will ignore %s",name);
    push_lit(ctx->ignore_list, name);
    return false;
}

bool source_all(const char *ext){
    ctx->extension = (char*)ext;
    find_files((char*)ext);
    return false;
}

void debug(){
    ctx->debug_syms = true;
}

void rebuild_self(bool emit_cc){
    new_module("redbuild");
    set_name("build");
    
    quick_cred("build.redb", "build.c");
    
    set_target(target_native, true);
    set_package_type(package_bin);
    
    debug();
    
    source("build.c");
    
	system("mv build build.old");
    if (compile()){
	    if (emit_cc) { gen_compile_commands("build.c"); }
		system("rm -f build.old");
    } else system("mv build.old build");
}

bool redbuild_run(const char *directory){
    buffer b = buffer_create(256, buffer_can_grow);
    buffer_write(&b, "(cd %s; ./build -t %s)",directory,target_to_string(global_target));
    redbuild_debug("Final redc command %s",b.buffer);
    return system(b.buffer) == 0;
}

bool git_sync(){
    return system("git pull") == 0;
}
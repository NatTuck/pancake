
#include <stdlib.h>
#include <assert.h>
#include <gc.h>

#include <drip/lstring.h>
#include <drip/lio.h>

#include "pancake/spec.h"

static
void
finalize_program_info(void* info_ptr, void* _unused)
{
    pancake_program_info* info = (pancake_program_info*) info_ptr;
    json_decref(info->json);
}

static
void
finalize_kernel_info(void* info_ptr, void* _unused)
{
    pancake_kernel_info* info = (pancake_kernel_info*) info_ptr;
    json_decref(info->json);
}

static
char*
pancake_path()
{
    char* path = getenv("PANCAKE");

    if (path == 0) {
        fprintf(stderr, "Try setting the PANCAKE variable to the pancake path.\n");
        fflush(stderr);
        abort();
    }

    return lstrdup(path);
}

pancake_program_info*
pancake_analyze_program(const char* filename)
{
    pancake_program_info* info = GC_malloc(sizeof(pancake_program_info));
    GC_register_finalizer((void*) info, finalize_program_info, 0, 0, 0);

    char* pancake_base = pancake_path();

    char* cmd = lsprintf("perl %s/scripts/analyze_program.pl %s",
            pancake_base, filename);

    char* info_json = lshell(cmd);
    json_error_t error;

    info->json = json_loads(info_json, 0, &error);
    if (!info->json) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        fprintf(stderr, "JSON text: %s\n", info_json);
        fflush(stderr);
        abort();
    }

    return info;
}

pancake_kernel_info*
pancake_get_kernel_info(pancake_program_info* prog_info, char* kernel_name)
{
    json_t* info = prog_info->json;
    assert(json_is_array(info));

    for (int ii = 0; ii < json_array_size(info); ++ii) {
        json_t* ki = json_array_get(info, ii);
        assert(json_is_object(ki));
        
        json_t* name_js = json_object_get(ki, "name");
        assert(json_is_string(name_js));

        const char* name = json_string_value(name_js);

        if (streq(kernel_name, name)) {
            pancake_kernel_info* info = GC_malloc(sizeof(pancake_kernel_info));
            GC_register_finalizer((void*) info, finalize_kernel_info, 0, 0, 0);

            info->json = ki;
            json_incref(ki);

            return info;
        }
    }

    fprintf(stderr, "No such kernel: %s\n", kernel_name);
    fflush(stderr);
    abort();
}

char*
pancake_kernel_name(pancake_kernel_info* kern_info)
{
    json_t* info = kern_info->json;
    assert(json_is_object(info));
    json_t* name = json_object_get(info, "name");
    assert(json_is_string(name));
    return lstrdup(json_string_value(name));
}

size_t
pancake_kernel_num_args(pancake_kernel_info* kern_info)
{
    json_t* info = kern_info->json;
    assert(json_is_object(info));
    json_t* args = json_object_get(info, "args");
    assert(json_is_array(args));
    return json_array_size(args);
}

char*
pancake_kernel_arg_name(pancake_kernel_info* kern_info, int ii)
{
    json_t* info = kern_info->json;
    assert(json_is_object(info));
    json_t* args = json_object_get(info, "args");
    assert(json_is_array(args));
    json_t* arg  = json_array_get(args, ii);
    assert(json_is_object(arg));
    json_t* name = json_object_get(arg, "name");
    assert(json_is_string(name));
    return lstrdup(json_string_value(name));
}
    
char*
pancake_kernel_arg_type(pancake_kernel_info* kern_info, int ii)
{
    json_t* info = kern_info->json;
    assert(json_is_object(info));
    json_t* args = json_object_get(info, "args");
    assert(json_is_array(args));
    json_t* arg  = json_array_get(args, ii);
    assert(json_is_object(arg));
    json_t* type = json_object_get(arg, "type");
    assert(json_is_string(type));
    return lstrdup(json_string_value(type));
}

int
pancake_kernel_arg_spec(pancake_kernel_info* kern_info, int ii)
{
    json_t* info = kern_info->json;
    assert(json_is_object(info));
    json_t* args = json_object_get(info, "args");
    assert(json_is_array(args));
    json_t* arg  = json_array_get(args, ii);
    assert(json_is_object(arg));
    json_t* type = json_object_get(arg, "spec");
    return json_is_true(type);
}

void
pancake_print_kernel_info(pancake_kernel_info* info)
{
    printf("== pancake_print_kernel_info ==\n");
    printf("Kernel name: %s\n", pancake_kernel_name(info));

    size_t nn = pancake_kernel_num_args(info);
    printf(" (%ld args):\n", nn);
    
    for (int ii = 0; ii < nn; ++ii) {
        char* name = pancake_kernel_arg_name(info, ii);
        char* type = pancake_kernel_arg_type(info, ii);
        int   spec = pancake_kernel_arg_spec(info, ii);
        printf("%s: %s (spec = %d)\n", name, type, spec); 
    }
}

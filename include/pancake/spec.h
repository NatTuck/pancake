#ifndef PANCAKE_SPEC_H
#define PANCAKE_SPEC_H

#include <jansson.h>

typedef struct pancake_program_info {
    json_t* json;
} pancake_program_info;

typedef struct pancake_kernel_info {
    json_t* json;
} pancake_kernel_info;

pancake_program_info* pancake_analyze_program(const char* filename);
pancake_kernel_info*  pancake_get_kernel_info(
        pancake_program_info* prog_info, char* kernel_name);
void pancake_print_kernel_info(pancake_kernel_info* info);

char* pancake_kernel_spec_filename(pancake_kernel_info* kern_info);

char* pancake_kernel_name(pancake_kernel_info* kern_info);
size_t pancake_kernel_num_args(pancake_kernel_info* kern_info);

char* pancake_kernel_arg_name(pancake_kernel_info* kern_info, int ii);
char* pancake_kernel_arg_type(pancake_kernel_info* kern_info, int ii);
int   pancake_kernel_arg_spec(pancake_kernel_info* kern_info, int ii);
char* pancake_kernel_arg_value(pancake_kernel_info* kern_info, int ii);
void  pancake_kernel_arg_set_value(pancake_kernel_info* kern_info, int ii, const char* vv);

void pancake_kernel_specialize(pancake_kernel_info* kern_info, const char* src, const char* dst);
#endif

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

char* pancake_kernel_name(pancake_kernel_info* kern_info);
#endif

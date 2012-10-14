#ifndef PANCAKE_PARSER_H
#define PANCAKE_PARSER_H

/*
 * This file defines functions for translating Pancake-annotated OpenCL 
 * code into an intermediate form suitable for JIT compilation and
 * specialization.
 *
 * The Clang/LLVM API are used internally, but C compatibility is preserved
 * in this header for consistency.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pancake_module {
    void* module;
    char* asm_text;
} pancake_module;

pancake_module* pancake_create_module(const char* text);
char* pancake_module_get_asm(pancake_module* module);
void pancake_destroy_module(pancake_module* module);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

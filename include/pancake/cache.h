#ifndef PANCAKE_KERNEL_CACHE_H
#define PANCAKE_KERNEL_CACHE_H

#include <CL/cl.h>
#include <gc/gc.h>

typedef struct pancake_kernel_cache {
    char* key;
    cl_kernel kernel;
    struct pancake_kernel_cache* next;
} pancake_kernel_cache;

pancake_kernel_cache* pancake_cache_add(pancake_kernel_cache* cc, char* key, cl_kernel kk);
cl_kernel pancake_cache_get(pancake_kernel_cache* cc, char* key);


#endif

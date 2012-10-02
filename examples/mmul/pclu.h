#ifndef PCLU_H
#define PCLU_H

#include <CL/cl.h>

#include "pclu_perror.h"

#define pclu_check_call(msg,code) pclu_check_call_real((msg),(code), __FILE__, __LINE__)

/* pclu_context
 * 
 * Represents a device and associated command queue.
 *
 */

typedef struct pclu_context {
    cl_platform_id   platform;
    cl_device_id     device;
    cl_context       context;
    cl_command_queue queue;
    char*            info;
} pclu_context;

pclu_context* pclu_create_context();
void pclu_destroy_context(pclu_context* pclu);
char* pclu_context_info(pclu_context* pclu);

/* pclu_buffer
 *
 * Represents a block of data to be operated on by OpenCL
 * kernels.
 *
 */

typedef struct pclu_buffer {
    pclu_context* pclu;
    cl_mem        data;
    size_t        size;
} pclu_buffer;

pclu_buffer* pclu_create_buffer(pclu_context* pclu, size_t size);
void pclu_destroy_buffer(pclu_buffer* buf);
void pclu_write_buffer(pclu_buffer* buf, size_t data_size, void* data);
void pclu_read_buffer(pclu_buffer* buf, size_t data_size, void* data);

/* pclu_range
 *
 * Represents a 1, 2, or 3D index space for parallel execution.
 *
 */

typedef struct pclu_range {
    size_t nd;
    size_t global[3];
    /* size_t local[3]; */
} pclu_range;

pclu_range pclu_range_1d(size_t cols);
pclu_range pclu_range_2d(size_t cols, size_t rows);
pclu_range pclu_range_3d(size_t deep, size_t cols, size_t rows);

/* pclu_program
 *
 * Represents several OpenCL kernels loaded from a single source file.
 *
 */

typedef struct pclu_program {
    pclu_context* pclu;
    cl_program    program;
    char*         build_log;
    cl_uint       num_kernels;
    cl_kernel*    kernels;
} pclu_program;

pclu_program* pclu_create_program(pclu_context* pclu, const char* path);
void pclu_destroy_program(pclu_program* pgm);
char* pclu_program_build_log(pclu_program* pgm);

void pclu_call_kernel(pclu_program* pgm, const char* name, pclu_range range, size_t argc, ...);

#define buf_arg(xx) sizeof(cl_mem),(&(xx->data))
#define lit_arg(xx) sizeof(xx),(&xx)

/*
 * Random untility functions.
 *
 */

char* pclu_slurp_file(const char* path);

#endif

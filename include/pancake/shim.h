#ifndef PANCAKE_SHIM_H
#define PANCAKE_SHIM_H

#include <CL/cl.h>

/*
 * The following things need to be proxied to transparently
 * handle JIT specialization of OpenCL kernels:
 *
 * - Program compilation (must be delayed)
 * - Kernel creation (must be delayed)
 * - Kernel calls (must do compilation)
 *   - Setting kernel arguments (specialization values).
 * - Creating buffers (specializaton values).
 *
 * This means that every function that operates on these data
 * types needs to be proxied as well:
 *
 * - Programs
 * - Kernels
 * - Buffers
 * 
 * We can probably skip buffers if we're only specializing
 * on scalar values.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "pancake/spec.h"

/* pancake_cl_program - 
 * 
 * Proxy struct for cl_program
 *
 */

typedef struct pancake_module pancake_module;

typedef struct pancake_cl_program_ {
    cl_program program;
    char* pancake_path;
    char* build_options;
    char* temp_dir;
    char* temp_source;
    pancake_program_info* info;
} pancake_cl_program_;

typedef pancake_cl_program_* pancake_cl_program;

pancake_cl_program pancake_clCreateProgramWithSource(cl_context context, cl_uint count, 
    const char** strings, const size_t* lengths, cl_int* errcode_return);
cl_int pancake_clRetainProgram(pancake_cl_program program);
cl_int pancake_clReleaseProgram(pancake_cl_program program);
cl_int pancake_clBuildProgram(pancake_cl_program program, cl_uint num_devices, 
    const cl_device_id* device_list, const char* options, 
    void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *user_data);
cl_int pancake_clGetProgramInfo(pancake_cl_program program, cl_program_info param_name,
    size_t param_value_size, void* param_value, size_t* param_value_size_ret);
cl_int pancake_clGetProgramBuildInfo (pancake_cl_program program, cl_device_id device,
    cl_program_build_info param_name, size_t param_value_size, void *param_value,
    size_t *param_value_size_ret);

#ifndef PANCAKE_INTERNAL
#define cl_program pancake_cl_program
#define clCreateProgramWithSource(a,b,c,d,e) pancake_clCreateProgramWithSource((a),(b),(c),(d),(e))
#define clRetainProgram(p) pancake_clRetainProgram((p))
#define clReleaseProgram(p) pancake_clReleaseProgram((p))
#define clBuildProgram(a,b,c,d,e,f) pancake_clBuildProgram((a),(b),(c),(d),(e),(f))
#define clGetProgramInfo(a,b,c,d,e) pancake_clGetProgramInfo((a),(b),(c),(d),(e))
#define clGetProgramBuildInfo(a,b,c,d,e,f) pancake_clGetProgramBuildInfo((a),(b),(c),(d),(e),(f))
#endif

/* pancake_cl_kernel -
 *
 * Proxy struct for cl_kernel 
 *
 */

typedef struct pancake_cl_kernel_ {
    cl_kernel kernel;
    char*   name;
    size_t  num_args;
    size_t* arg_size;
    const void** arg_value;
    pancake_kernel_info* info;
} pancake_cl_kernel_;

typedef pancake_cl_kernel_* pancake_cl_kernel;

pancake_cl_kernel pancake_clCreateKernel(pancake_cl_program program, const char *kernel_name, 
    cl_int *errcode_ret);
cl_int pancake_clCreateKernelsInProgram (pancake_cl_program program, cl_uint num_kernels, 
    pancake_cl_kernel *kernels, cl_uint *num_kernels_ret);
cl_int pancake_clRetainKernel(pancake_cl_kernel kernel);
cl_int pancake_clReleaseKernel(pancake_cl_kernel kernel);
cl_int pancake_clSetKernelArg (pancake_cl_kernel kernel, cl_uint arg_index, 
    size_t arg_size, const void *arg_value);
cl_int pancake_clEnqueueNDRangeKernel (cl_command_queue command_queue, pancake_cl_kernel kernel,
    cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size,
    const size_t *local_work_size, cl_uint num_events_in_wait_list, 
    const cl_event *event_wait_list, cl_event *event);


#ifndef PANCAKE_INTERNAL
#define cl_kernel pancake_cl_kernel
#define clCreateKernel(a,b,c) pancake_clCreateKernel((a),(b),(c))
#define clCreateKernelsInProgram(a,b,c,d) pancake_clCreateKernelsInProgram((a),(b),(c),(d))
#define clRetainKernel(k) pancake_clRetainKernel((k))
#define clReleaseKernel(k) pancake_clReleaseKernel((k))
#define clSetKernelArg(a,b,c,d) pancake_clSetKernelArg((a),(b),(c),(d))
#define clEnqueueNDRangeKernel(a,b,c,d,e,f,g,h,i) pancake_clEnqueueNDRangeKernel( \
	(a),(b),(c),(d),(e),(f),(g),(h),(i))
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif

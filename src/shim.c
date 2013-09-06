
#include <stdio.h>
#include <assert.h>
#include <alloca.h>
#include <string.h>

#define PANCAKE_INTERNAL
#include <pancake/shim.h>

pancake_cl_program 
pancake_clCreateProgramWithSource(cl_context context, cl_uint count, 
    const char** strings, const size_t* lengths, cl_int* errcode_return)
{
    pancake_cl_program pgm = (pancake_cl_program) malloc(sizeof(pancake_cl_program_));
    pgm->refs = 1;
    pgm->build_options = 0;

    /* Need to create unspecialized program here */
    pgm->program = clCreateProgramWithSource(context, count, strings, lengths, errcode_return);

    /* Write source to disk */
    pgm->tmpdir = sprintf

    return pgm;
}

cl_int 
pancake_clRetainProgram(pancake_cl_program program)
{
    program->refs += 1;
    return CL_SUCCESS;
}

cl_int 
pancake_clReleaseProgram(pancake_cl_program program)
{
    program->refs -= 1;

    if (program->refs > 0)
	return CL_SUCCESS;

    if (program-> refs < 0) {
        fprintf(stderr, "pancake_clReleaseProgram: called when refs <= 0");
        exit(1);
    }

    int errcode = clReleaseProgram(program->program);

    if (program->build_options)
        free(program->build_options);

    free(program);
    
    return errcode;
}

cl_int 
pancake_clBuildProgram(pancake_cl_program program, cl_uint num_devices, 
    const cl_device_id* device_list, const char* options, 
    void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *user_data)
{
    /* Save some parameters */
    if (program->build_options)
        free(program->build_options);

    program->build_options = strdup(options);

    /* Build generic version of program to catch errors and generate
     * expected program state. */
    return clBuildProgram(program->program, num_devices, device_list, options,
			  pfn_notify, user_data);
}

cl_int
pancake_clGetProgramInfo(pancake_cl_program program, cl_program_info param_name,
    size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    fprintf(stderr, "TODO: pancake_clGetPrograminfo\n");
    exit(1);
}

cl_int 
pancake_clGetProgramBuildInfo(pancake_cl_program program, cl_device_id device,
    cl_program_build_info param_name, size_t param_value_size, void *param_value,
    size_t *param_value_size_ret)
{
    return clGetProgramBuildInfo(program->program, device, param_name, param_value_size,
				 param_value, param_value_size_ret);
}

pancake_cl_kernel 
pancake_clCreateKernel(pancake_cl_program program, const char *kernel_name, cl_int *errcode_ret)
{
    pancake_cl_kernel kk = (pancake_cl_kernel) malloc(sizeof(pancake_cl_kernel_));
    kk->refs   = 1;
    kk->kernel = clCreateKernel(program->program, kernel_name, errcode_ret);
    return kk;
}

cl_int 
pancake_clCreateKernelsInProgram(pancake_cl_program program, cl_uint num_kernels,
    pancake_cl_kernel *kernels, cl_uint *num_kernels_ret)
{
    /* Are we just reserving space? */
    if (num_kernels == 0)
	return clCreateKernelsInProgram(program->program, 0, 0, num_kernels_ret);

    assert(num_kernels_ret == 0 && "when num_kernels != 0");

    /* No, let's build some kernel structures. */
    cl_kernel* ks = (cl_kernel*) alloca(num_kernels * sizeof(cl_kernel));

    int errcode = clCreateKernelsInProgram(program->program, num_kernels, ks, 0);

    for (cl_uint ii = 0; ii < num_kernels; ++ii) {
        pancake_cl_kernel kk = (pancake_cl_kernel) malloc(sizeof(pancake_cl_kernel_));
        kk->refs   = 1;
        kk->kernel = ks[ii];
        kernels[ii] = kk;
    }

    return errcode;
}

cl_int 
pancake_clRetainKernel(pancake_cl_kernel kernel)
{
    kernel->refs += 1;
    return CL_SUCCESS;
}

cl_int 
pancake_clReleaseKernel(pancake_cl_kernel kernel)
{
    kernel->refs -= 1;

    if (kernel->refs > 0)
	return CL_SUCCESS;

    if (kernel->refs == 0) {
        int errcode = clReleaseKernel(kernel->kernel);
        free(kernel);
        return errcode;
    }

    fprintf(stderr, "pancake_clReleaseKernel: called when refs <= 0");
    exit(1);
}

cl_int 
pancake_clSetKernelArg(pancake_cl_kernel kernel, cl_uint arg_index, 
    size_t arg_size, const void *arg_value)
{
    printf("Set arg %d\n", arg_index);
    return clSetKernelArg(kernel->kernel, arg_index, arg_size, arg_value);
}

cl_int 
pancake_clEnqueueNDRangeKernel (cl_command_queue command_queue, pancake_cl_kernel kernel,
    cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size,
    const size_t *local_work_size, cl_uint num_events_in_wait_list, 
    const cl_event *event_wait_list, cl_event *event)
{
    return clEnqueueNDRangeKernel(command_queue, kernel->kernel, work_dim, 
            global_work_offset, global_work_size, local_work_size, num_events_in_wait_list,
            event_wait_list, event);
}

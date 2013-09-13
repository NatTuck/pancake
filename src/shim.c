
#include <stdio.h>
#include <assert.h>
#include <alloca.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <gc.h>

#include <drip/lstring.h>
#include <drip/lio.h>

#define PANCAKE_INTERNAL
#include <pancake/shim.h>
#include <pancake/spec.h>

typedef struct temp_dir_node {
    char* temp_dir;
    struct temp_dir_node* next;
} temp_dir_node;

static temp_dir_node* pancake_temp_dirs = 0;

static
void
add_temp_dir(char* dir)
{
    temp_dir_node* node = GC_malloc(sizeof(temp_dir_node));
    node->temp_dir = lstrdup(dir);
    node->next = pancake_temp_dirs;
    pancake_temp_dirs = node;
}

static
void
cleanup_temp_dirs()
{
    while (pancake_temp_dirs != 0) {
        temp_dir_node* node = pancake_temp_dirs;
        char* dir  = node->temp_dir;
        if (strncmp(dir, "/tmp", 4) == 0 &&
            strstr(dir, "pancake") != 0) {
            int rv = system(lsprintf("rm -rf %s", dir));
            assert(WEXITSTATUS(rv) == 0);
        }
        else {
            fprintf(stderr, "pancake cleanup: skipping bad dir: %s\n", dir);
        }

        pancake_temp_dirs = node->next;
    }
}

pancake_cl_program 
pancake_clCreateProgramWithSource(cl_context context, cl_uint count, 
    const char** strings, const size_t* lengths, cl_int* errcode_return)
{
    cl_int err;
    char*  cmd;

    pancake_cl_program program = (pancake_cl_program) GC_malloc(sizeof(pancake_cl_program_));
    program->build_options = 0;

    /* Need to create unspecialized program here */
    program->program = clCreateProgramWithSource(context, count, strings, lengths, errcode_return);

    /* Write source to disk */
    program->temp_dir = ltempname("pancake");
    add_temp_dir(program->temp_dir);
    atexit(cleanup_temp_dirs);

    cmd = lsprintf("mkdir -p %s", program->temp_dir);
    int rv = system(cmd);
    assert(WEXITSTATUS(rv) == 0);

    size_t source_size;
    err = clGetProgramInfo(program->program, CL_PROGRAM_SOURCE, 0, 0, &source_size);
    if (err != CL_SUCCESS) {
        *errcode_return = err;
        return 0;
    }

    char* src_text = GC_malloc(source_size);
    err = clGetProgramInfo(program->program, CL_PROGRAM_SOURCE, source_size, src_text, 0);
    if (err != CL_SUCCESS) {
        *errcode_return = err;
        return 0;
    }

    program->temp_source = lsprintf("%s/program.cl", program->temp_dir);
    ldump(program->temp_source, src_text);

    program->info = pancake_analyze_program(program->temp_source);

    return program;
}

cl_int 
pancake_clRetainProgram(pancake_cl_program program)
{
    return clRetainProgram(program->program);
}

cl_int 
pancake_clReleaseProgram(pancake_cl_program program)
{
    return clReleaseProgram(program->program);
}

cl_int 
pancake_clBuildProgram(pancake_cl_program program, cl_uint num_devices, 
    const cl_device_id* device_list, const char* options, 
    void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *user_data)
{
    /* Save some parameters */
    program->build_options = lstrdup(options);

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
    fflush(stderr);
    abort();
}

cl_int 
pancake_clGetProgramBuildInfo(pancake_cl_program program, cl_device_id device,
    cl_program_build_info param_name, size_t param_value_size, void *param_value,
    size_t *param_value_size_ret)
{
    return clGetProgramBuildInfo(program->program, device, param_name, param_value_size,
				 param_value, param_value_size_ret);
}

static
cl_uint
kernel_get_num_args(cl_kernel kernel)
{
    cl_uint nargs;
    cl_int  error;

    error = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(nargs), &nargs, 0);

    if (error != CL_SUCCESS) {
        fprintf(stderr, "clGetKernelInfo(NUM_ARGS) error %d\n", error);
        fflush(stderr);
        abort();
    }

    return nargs;
}

static
char*
kernel_get_name(cl_kernel kernel)
{
    size_t size;
    cl_int error;

    error = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 0, 0, &size);
    
    if (error != CL_SUCCESS) {
        fprintf(stderr, "clGetKernelInfo(NAME) (1) error %d\n", error);
        fflush(stderr);
        abort();
    }

    char* name = (char*) GC_malloc(size);

    error = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, size, name, 0);
    
    if (error != CL_SUCCESS) {
        fprintf(stderr, "clGetKernelInfo(NAME) (1) error %d\n", error);
        fflush(stderr);
        abort();
    }

    return name;
}

static
void
kernel_init(pancake_cl_program program, pancake_cl_kernel kk)
{
    kk->program   = program;
    kk->num_args  = kernel_get_num_args(kk->kernel);
    kk->name      = kernel_get_name(kk->kernel);
    kk->arg_size  = GC_malloc(kk->num_args * sizeof(size_t));
    kk->arg_value = GC_malloc(kk->num_args * sizeof(void*));
    kk->info      = pancake_get_kernel_info(program->info, kk->name);
}

pancake_cl_kernel 
pancake_clCreateKernel(pancake_cl_program program, const char *kernel_name, cl_int *errcode_ret)
{
    pancake_cl_kernel kk = (pancake_cl_kernel) GC_malloc(sizeof(pancake_cl_kernel_));
    kk->kernel = clCreateKernel(program->program, kernel_name, errcode_ret);
    kernel_init(program, kk);
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
        pancake_cl_kernel kk = (pancake_cl_kernel) GC_malloc(sizeof(pancake_cl_kernel_));
        kk->kernel = ks[ii];
        kernel_init(program, kk);
        kernels[ii] = kk;
    }

    return errcode;
}

cl_int 
pancake_clRetainKernel(pancake_cl_kernel kernel)
{
    return clRetainKernel(kernel->kernel);
}

cl_int 
pancake_clReleaseKernel(pancake_cl_kernel kernel)
{
    return clReleaseKernel(kernel->kernel);
}

cl_int 
pancake_clSetKernelArg(pancake_cl_kernel kernel, cl_uint ii,
    size_t arg_size, const void *arg_value)
{
    kernel->arg_size[ii] = arg_size;
    kernel->arg_value[ii] = arg_value;

    if (pancake_kernel_arg_spec(kernel->info, ii)) {
        char* type = pancake_kernel_arg_type(kernel->info, ii);

        if (streq(type, "long")) {
            cl_long vv = *((long*) arg_value);
            pancake_kernel_arg_set_value(kernel->info, ii, lsprintf("%ld", vv));
        }
        else {
            fprintf(stderr, "Error: Can't handle spec arg of type '%s'.\n", type);
            fflush(stderr);
            abort();
        }
    }

    return clSetKernelArg(kernel->kernel, ii, arg_size, arg_value);
}

cl_int 
pancake_clEnqueueNDRangeKernel (cl_command_queue command_queue, pancake_cl_kernel kernel,
    cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size,
    const size_t *local_work_size, cl_uint num_events_in_wait_list, 
    const cl_event *event_wait_list, cl_event *event)
{
    pancake_print_kernel_info(kernel->info);

    char* spec_filename = pancake_kernel_spec_filename(kernel->info);
    char* spec_filepath = lsprintf("%s/%s", kernel->program->temp_dir, spec_filename);

    pancake_kernel_specialize(kernel->info, kernel->program->temp_source, spec_filepath);

    return clEnqueueNDRangeKernel(command_queue, kernel->kernel, work_dim, 
            global_work_offset, global_work_size, local_work_size, num_events_in_wait_list,
            event_wait_list, event);
}

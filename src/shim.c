
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
#include <pancake/timer.h>
#include <pancake/cache.h>

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

char*
pancake_status()
{
    char *status = lstrdup("");

    if (getenv("PANCAKE_SPEC")) {
        status = lstrcat(status, "enabled ");
    }
    else {
        return lstrdup("disabled");
    }

    if (getenv("PANCAKE_NOSPEC")) {
        status = lstrcat(status, "no-spec ");
    }
    else {
        status = lstrcat(status, "spec ");
    }

    if (getenv("PANCAKE_UNROLL")) {
        status = lstrcat(status, "unroll");
    }
    else {
        status = lstrcat(status, "no-unroll");
    }

    return status;
}

pancake_cl_program 
pancake_clCreateProgramWithSource(cl_context context, cl_uint count, 
    const char** strings, const size_t* lengths, cl_int* errcode_return)
{
    cl_int err;
    char*  cmd;

    pancake_cl_program program = (pancake_cl_program) GC_malloc(sizeof(pancake_cl_program_));
    program->build_options = 0;
    program->context = context;

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

    if (getenv("PANCAKE_STATUS")) {
        cl_uint ndv;
        clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(ndv), &ndv, 0);

        if (ndv == 0) {
            fprintf(stderr, "WTF?!?!\n");
            fflush(stderr);
            abort();
        }

        cl_uint  dvs_size = sizeof(cl_device_id) * ndv;
        cl_device_id* dvs = alloca(dvs_size);
        clGetContextInfo(context, CL_CONTEXT_DEVICES, dvs_size, dvs, 0);

        FILE* ff = fopen(getenv("PANCAKE_STATUS"), "w");
        if (ff == 0) {
            perror("gargh!");
            fflush(stderr);
            abort();
        }

        fprintf(ff, "pancake: %s\n", pancake_status());

        char* dname = alloca(512);

        for(int ii = 0; ii < ndv; ++ii) {
            clGetDeviceInfo(dvs[ii], CL_DEVICE_NAME, 512, (void*)dname, 0);
            fprintf(ff, "device: %s\n", dname);
        }

        fclose(ff);
    }

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
    program->build_options = lstrdup(options ? options : "");
    program->num_devices   = num_devices;

    if (num_devices > 0) {
        program->device_list = lmemcpy(device_list, num_devices * sizeof(cl_device_id*));
    }
    else {
        program->device_list = 0;
    }

    /* Build generic version of program to catch errors and generate
     * expected program state. */
    return clBuildProgram(program->program, num_devices, device_list, options,
			  pfn_notify, user_data);
}

cl_int
pancake_clGetProgramInfo(pancake_cl_program program, cl_program_info param_name,
    size_t param_value_size, void* param_value, size_t* param_value_size_ret)
{
    return clGetProgramInfo(program->program, param_name, param_value_size, 
            param_value, param_value_size_ret);
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
        else if (streq(type, "int")) {
            cl_int vv = *((int*) arg_value);
            pancake_kernel_arg_set_value(kernel->info, ii, lsprintf("%d", vv));
        }
        else if (streq(type, "float")) {
            float vv = *((float*) arg_value);
            pancake_kernel_arg_set_value(kernel->info, ii, lsprintf("%f", vv));
        }
        else {
            fprintf(stderr, "Error: Can't handle spec arg of type '%s'.\n", type);
            fflush(stderr);
            abort();
        }
    }

    return clSetKernelArg(kernel->kernel, ii, arg_size, arg_value);
}

static
cl_kernel
get_spec_kernel(pancake_cl_kernel gen_kern, const char* filename, const char* kern_name)
{
    cl_int errcode;
    char* program_text  = lslurp(filename);
    size_t program_size = strlen(program_text);

    const char** sources = (const char **) &program_text;

    cl_program program = clCreateProgramWithSource(gen_kern->program->context, 1, 
            sources, &program_size, &errcode);
    assert(errcode == CL_SUCCESS);

    errcode = clBuildProgram(program, gen_kern->program->num_devices, 
            gen_kern->program->device_list, gen_kern->program->build_options, 0, 0);

    /* Print out errors on failure */
    if (errcode != CL_SUCCESS) {
        fprintf(stderr, "Got error in clBuildProgram for spec kernel '%s'.\n", kern_name);
        if (errcode == CL_BUILD_PROGRAM_FAILURE)
            fprintf(stderr, "Error was CL_BUILD_PROGRAM_FAILURE\n");
        else 
            fprintf(stderr, "Error was %d\n", errcode);

        size_t log_size;
        char*  log_text;

        errcode = clGetProgramBuildInfo(program, gen_kern->program->device_list[0], 
                CL_PROGRAM_BUILD_LOG, 0, 0, &log_size);
        assert(errcode == CL_SUCCESS);

        log_text = (char*) alloca(log_size);

        errcode = clGetProgramBuildInfo(program, gen_kern->program->device_list[0], 
                CL_PROGRAM_BUILD_LOG, log_size, log_text, 0);
        assert(errcode == CL_SUCCESS);

        fprintf(stderr, "Build Errors\n%s\n", log_text);
        fflush(stderr);
        abort();
    }

    cl_kernel kern = clCreateKernel(program, kern_name, &errcode);
    assert(errcode == CL_SUCCESS);

    return kern;
}

cl_int 
pancake_clEnqueueNDRangeKernel (cl_command_queue command_queue, pancake_cl_kernel kernel,
    cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size,
    const size_t *local_work_size, cl_uint num_events_in_wait_list, 
    const cl_event *event_wait_list, cl_event *event)
{
    static pancake_kernel_cache* cache = 0;

    int rv;

    //pancake_print_kernel_info(kernel->info);

    cake_timer tt;

    if (getenv("PANCAKE_SPEC")) {
        char* spec_filename = pancake_kernel_spec_filename(kernel->info);
        char* spec_filepath = lsprintf("%s/%s", kernel->program->temp_dir, spec_filename);

        cake_timer_reset(&tt);

        cl_kernel spec_kern = pancake_cache_get(cache, spec_filepath);

        if (spec_kern == 0) {
            pancake_kernel_specialize(kernel->info, kernel->program->temp_source, spec_filepath);
            spec_kern = get_spec_kernel(kernel, spec_filepath, kernel->name);
            cache = pancake_cache_add(cache, spec_filepath, spec_kern);
        }

        cake_timer_log(&tt, kernel->name, "opt");
        
        cake_timer_reset(&tt);

        for (int ii = 0; ii < kernel->num_args; ++ii) {
            int errcode = clSetKernelArg(spec_kern, ii, kernel->arg_size[ii], kernel->arg_value[ii]);
            assert(errcode == CL_SUCCESS);
        }

        rv = clEnqueueNDRangeKernel(command_queue, spec_kern, work_dim, global_work_offset,
                global_work_size, local_work_size, num_events_in_wait_list, event_wait_list,
                event);

        clFinish(command_queue);
        
        cake_timer_log(&tt, kernel->name, "run");
    }
    else {
        
        cake_timer_reset(&tt);

        cake_timer_log(&tt, kernel->name, "opt");
        
        cake_timer_reset(&tt);

        rv = clEnqueueNDRangeKernel(command_queue, kernel->kernel, work_dim, 
                global_work_offset, global_work_size, local_work_size, num_events_in_wait_list,
                event_wait_list, event);

        clFinish(command_queue);
        
        cake_timer_log(&tt, kernel->name, "run");
    }

    return rv;
}

cl_int
pancake_clGetKernelWorkGroupInfo(pancake_cl_kernel kernel, cl_device_id device,
 	cl_kernel_work_group_info param_name, size_t param_value_size, void *param_value,
 	size_t *param_value_size_ret)
{
    return clGetKernelWorkGroupInfo(kernel->kernel, device, param_name, param_value_size,
            param_value, param_value_size_ret);
}

cl_int 
pancake_clEnqueueTask(cl_command_queue command_queue, pancake_cl_kernel kernel, 
    cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
    size_t size[] = {1, 0, 0};
    return pancake_clEnqueueNDRangeKernel(command_queue, kernel, 1, 0, size, size,
            num_events_in_wait_list, event_wait_list, event);
}



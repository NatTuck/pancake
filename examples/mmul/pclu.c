
#include <stdio.h>
#include <alloca.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include <pancake/shim.h>

#include "pclu.h"

void
pclu_check_call_real(const char* name, int error_code, const char* file, const int line)
{
    if (error_code != CL_SUCCESS) {
	fprintf(stderr, "Error at %s:%d in call to %s:\n  ", file, line, name);
	pclu_perror(error_code);
	exit(1);
    }
}

static
void
pclu_find_best_device(pclu_context* pclu)
{
    const char* dev_type_str = getenv("OPENCL_DEVICE_TYPE");
    cl_uint dev_type = CL_DEVICE_TYPE_DEFAULT;

    if(dev_type_str) {
	if (dev_type_str[0] == 'C')
	    dev_type = CL_DEVICE_TYPE_CPU;
	if (dev_type_str[0] == 'G')
	    dev_type = CL_DEVICE_TYPE_GPU;
    }

    cl_uint num_platforms;
    pclu_check_call("clGetPlatformIDs", clGetPlatformIDs(0, 0, &num_platforms));

    cl_platform_id* platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id)*num_platforms);
    pclu_check_call("clGetPlatformIDs", clGetPlatformIDs(num_platforms, platforms, 0));

    for (size_t ii = 0; ii < num_platforms; ++ii) {
	cl_uint num_devices;
	pclu_check_call("clGetDeviceIDs", 
			clGetDeviceIDs(platforms[ii], dev_type, 0, 0, &num_devices));

	if (num_devices > 0) {
	    cl_device_id* devices = (cl_device_id*) malloc(sizeof(cl_device_id)*num_devices);
	    pclu_check_call("clGetDeviceIDs", 
			    clGetDeviceIDs(platforms[ii], dev_type, num_devices, devices, 0));

	    pclu->platform = platforms[ii];
	    pclu->device   = devices[0];

	    free(devices);
	    free(platforms);

	    return;
	}
    }

    fprintf(stderr, "No valid OpenCL device found.\n");
    exit(1);
}

void
pclu_error_callback(const char* errinfo, const void* _x, size_t _y, void* _z)
{
    fprintf(stderr, "OpenCL Error (callback): %s\n", errinfo);
    exit(3);
}

pclu_context*
pclu_create_context()
{
    int errcode;

    pclu_context* pclu = (pclu_context*) malloc(sizeof(pclu_context));

    pclu_find_best_device(pclu);

    long props[] = {CL_CONTEXT_PLATFORM, (long)pclu->platform, 0};
    pclu->context = clCreateContext((cl_context_properties*) props, 1, 
				    &(pclu->device), pclu_error_callback, 0, &errcode);
    pclu_check_call("clCreateContext", errcode);

    pclu->queue = clCreateCommandQueue(pclu->context, pclu->device, 0, &errcode);
    pclu_check_call("clCreateCommandQueue", errcode);

    pclu->info = 0;

    return pclu;
}

void
pclu_destroy_context(pclu_context* pclu)
{
    pclu_check_call("clReleaseCommandQueue", clReleaseCommandQueue(pclu->queue));
    pclu_check_call("clReleaseContext",      clReleaseContext(pclu->context));

    if (pclu->info)
	free(pclu->info);

    free(pclu);
}

char*
pclu_context_info(pclu_context* pclu)
{
    size_t pclu_status_size = 32; // Add some extra space for label text.

    char*  platform_name;
    size_t platform_name_size;

    char*  device_name;
    size_t device_name_size;

    if (pclu->info)
	return pclu->info;

    pclu_check_call("clGetPlatformInfo", 
		    clGetPlatformInfo(pclu->platform, CL_PLATFORM_NAME, 
				      0, 0, &platform_name_size));
    pclu_status_size += platform_name_size;

    platform_name = (char*) alloca(platform_name_size);
    pclu_check_call("clGetPlatformInfo",
		    clGetPlatformInfo(pclu->platform, CL_PLATFORM_NAME, 
				      platform_name_size, platform_name, 0));

    pclu_check_call("clGetDeviceInfo",
		    clGetDeviceInfo(pclu->device, CL_DEVICE_NAME,
				    0, 0, &device_name_size));
    pclu_status_size += device_name_size;

    device_name = (char*) alloca(device_name_size);
    pclu_check_call("clGetDeviceInfo",
		    clGetDeviceInfo(pclu->device, CL_DEVICE_NAME,
				    device_name_size, device_name, 0));

    pclu->info = (char*) malloc(pclu_status_size);
    snprintf(pclu->info, pclu_status_size, "Platform:\t%s\nDevice: \t%s\n", 
	     platform_name, device_name);

    return pclu->info;
}

pclu_buffer* 
pclu_create_buffer(pclu_context* pclu, size_t size)
{
    cl_mem_flags flags = CL_MEM_READ_WRITE;
    cl_int errcode;

    pclu_buffer* buf = (pclu_buffer*) malloc(sizeof(pclu_buffer));
    buf->pclu = pclu;
    buf->size = size;

    buf->data = clCreateBuffer(pclu->context, flags, size, 0, &errcode);
    pclu_check_call("clCreateBuffer", errcode);

    return buf;
}

void
pclu_destroy_buffer(pclu_buffer* buf)
{
    pclu_check_call("clReleaseMemObject", clReleaseMemObject(buf->data));
    free(buf);
}

void
pclu_write_buffer(pclu_buffer* buf, size_t data_size, void* data)
{
    assert(data_size == buf->size && "size must match for write_buffer");

    pclu_check_call("clEnqueueWriteBuffer",
		    clEnqueueWriteBuffer(buf->pclu->queue, buf->data, CL_TRUE, 0, 
					 data_size, data, 0, 0, 0));
}

void
pclu_read_buffer(pclu_buffer* buf, size_t data_size, void* data)
{
    assert(data_size == buf->size && "size must match for read_buffer");

    pclu_check_call("clEnqueueReadBuffer",
		    clEnqueueReadBuffer(buf->pclu->queue, buf->data, CL_TRUE, 0, 
					data_size, data, 0, 0, 0));
}

pclu_range 
pclu_range_1d(size_t cols)
{
    pclu_range range;
    range.nd = 1;
    range.global[0] = cols;
    return range;
}

pclu_range 
pclu_range_2d(size_t cols, size_t rows)
{
    pclu_range range;
    range.nd = 2;
    range.global[0] = cols;
    range.global[1] = rows;
    return range;
}

pclu_range 
pclu_range_3d(size_t deep, size_t cols, size_t rows)
{
    pclu_range range;
    range.nd = 3;
    range.global[0] = cols;
    range.global[1] = rows;
    range.global[3] = deep;
    return range;
}

char*
pclu_slurp_file(const char* path)
{
    const size_t BLOCK = 64 * 1024;

    size_t capacity = BLOCK;
    size_t size     = 0;
    char*  data     = (char*) malloc(capacity);
    FILE*  file     = fopen(path, "r");

    if (file == NULL) {
	perror("Could not open file");
	exit(1);
    }
    
    size_t count = 0;

    while ( (count = fread(data + size, 1, BLOCK, file)) ) {
	size += count;

	if (capacity < (size + BLOCK + 1)) {
	    capacity *= 2;
	    data = (char*) realloc(data, capacity);
	}
    }

    if (ferror(file)) {
	perror("read error");
	exit(1);
    }

    fclose(file);

    data[size] = 0;
    size += 1;

    return (char*) realloc(data, size);
}

pclu_program* 
pclu_create_program(pclu_context* pclu, const char* path)
{
    int errcode;

    pclu_program* pgm = (pclu_program*) malloc(sizeof(pclu_program));
    pgm->pclu      = pclu;
    pgm->build_log = 0;

    /* Read the source from disk */
    char* source = pclu_slurp_file(path);
    size_t  size = strlen(source);

    const char** sources = (const char**) &source;

    pgm->program = clCreateProgramWithSource(pclu->context, 1, sources, &size, &errcode);
    pclu_check_call("clCreateProgramWithSource", errcode);

    free(source);

    /* Compile for the device */

    pclu_check_call("clBuildProgram", 
		    clBuildProgram(pgm->program, 1, &(pclu->device), "", 0, 0));

    /* Get the kernels */

    pclu_check_call("clCreateKernelsInProgram",
		    clCreateKernelsInProgram(pgm->program, 0, 0, &(pgm->num_kernels)));

    pgm->kernels = (cl_kernel*) malloc(pgm->num_kernels*sizeof(cl_kernel));
    pclu_check_call("clCreateKernelsInProgram",
		    clCreateKernelsInProgram(pgm->program, pgm->num_kernels, pgm->kernels, 0));

    return pgm;
}

void 
pclu_destroy_program(pclu_program* pgm)
{
    if (pgm->build_log)
	free(pgm->build_log);

    for (size_t ii = 0; ii < pgm->num_kernels; ++ii) {
	pclu_check_call("clReleaseKernel", clReleaseKernel(pgm->kernels[ii]));
    }

    free(pgm->kernels);
    pclu_check_call("clReleaseProgram", clReleaseProgram(pgm->program));
    free(pgm);
}

char*
pclu_program_build_log(pclu_program* pgm)
{
    if (pgm->build_log)
	return pgm->build_log;

    size_t build_log_size;
    pclu_check_call("clGetProgramBuildInfo",
		    clGetProgramBuildInfo(pgm->program, pgm->pclu->device, 
					  CL_PROGRAM_BUILD_LOG, 0, 0, &build_log_size));

    pgm->build_log = (char*) malloc(build_log_size);
    pclu_check_call("clGetProgramBuildInfo",
		    clGetProgramBuildInfo(pgm->program, pgm->pclu->device, 
					  CL_PROGRAM_BUILD_LOG, build_log_size, 
					  pgm->build_log, 0));

    return pgm->build_log;
}

void
pclu_call_kernel(pclu_program* pgm, const char* name, pclu_range range, size_t argc, ...)
{
    cl_int errcode;
    cl_kernel kern = clCreateKernel(pgm->program, name, &errcode);
    pclu_check_call("clCreateKernel", errcode);

    va_list ap;
    va_start(ap, argc);

    for (cl_uint ii = 0; ii < argc; ++ii) {
	size_t size = va_arg(ap, size_t);	
	void*  arg  = va_arg(ap, void*);
	pclu_check_call("clSetKernelArg", clSetKernelArg(kern, ii, size, arg));
    }

    va_end(ap);

    cl_event kernel_done = clCreateUserEvent(pgm->pclu->context, &errcode);
    pclu_check_call("clCreateUserEvent", errcode);

    errcode = clEnqueueNDRangeKernel(pgm->pclu->queue, kern, range.nd, 0, 
				     range.global, 0, 0, 0, &kernel_done);
    pclu_check_call("clEnqueueNDRangeKernel", errcode);

    pclu_check_call("clWaitForEvents", clWaitForEvents(1, &kernel_done));

    pclu_check_call("clReleaseKernel", clReleaseKernel(kern));
}

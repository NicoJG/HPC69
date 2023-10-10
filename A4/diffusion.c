#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

#include "cmd_args.h"
#include "read_file.h"

const char *opencl_program_src = \
	" "\
	" "\
	" ";

int 
main(
    int argc,
    char *argv[]
) {
    short n_its = 20;
    float diff_const = 0.02;

    parse_cmd_args(argc, argv, &n_its, &diff_const);

    printf("Computing %d iterations with diffusion constant %f.\n", n_its, diff_const);
    cl_int error;

    cl_platform_id platform_id;
    cl_uint nmb_platforms;
    if ( clGetPlatformIDs(1, &platform_id, &nmb_platforms) != CL_SUCCESS ) {
	    fprintf(stderr, "cannot get platform\n" );
	    return 1;
    }

    cl_device_id device_id;
    cl_uint nmb_devices;
    if ( clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &nmb_devices) != CL_SUCCESS ) {
	    fprintf(stderr, "cannot get device\n" );
	    return 1;
    }
    cl_context context;
    cl_context_properties properties[] =
    {
	    CL_CONTEXT_PLATFORM,
	    (cl_context_properties) platform_id,
	    0
    };
    context = clCreateContext(properties, 1, &device_id, NULL, NULL, &error);
    if ( error != CL_SUCCESS ) {
	    fprintf(stderr, "cannot create context\n");
	    return 1;
    }

    cl_command_queue command_queue;
    command_queue = clCreateCommandQueueWithProperties(context, device_id, NULL, &error);
    if ( error != CL_SUCCESS ) {
	    fprintf(stderr, "cannot create command queue\n");
	    return 1;
    }
    
    cl_program program;
    size_t src_len = strlen(opencl_program_src);
    program = clCreateProgramWithSource(
		    context, 1, (const char **) &opencl_program_src, (const size_t*) &src_len, &error);
    if ( error != CL_SUCCESS ) {
	    fprintf(stderr, "cannot create program\n");
	    return 1;
    }
    
    // free(opencl_program_src);

    error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if ( error != CL_SUCCESS ) {
	    fprintf(stderr, "cannot build program. log:\n");
	    size_t log_size = 0;
	    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

	    char *log = malloc(log_size*sizeof(char));
	    if ( log == NULL ) {
		    fprintf(stderr, "could not allocate memory\n");
		    return 1;
	    }
	    
	    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

	    fprintf(stderr, "%s\n", log );

	    free(log);

	    return 1;
    }
    
    cl_kernel kernel = clCreateKernel(program, "dot_prod_mul", &error);
    if ( error != CL_SUCCESS ) {
	    fprintf(stderr, "cannot create kernel\n");
	    return 1;
    }

    clReleaseProgram(program);
    clReleaseKernel(kernel);

    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
    
    // read header 
    FILE *fp;
    fp = fopen("test_data/init_10000_1000", "r");

    int width, height;
    read_header(fp, &width, &height);

    printf("Width: %d\n", width);
    printf("Height: %d\n", height);

    fclose(fp);
    return 0;
    
}

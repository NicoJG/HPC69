#include <stdio.h>
#include <stddef.h>
#include <string.h>
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

#include "cmd_args.h"
#include "read_file.h"

int main(int argc, char *argv[]){
	
	// Start program
	printf("\n ---------- Diffusion ----------\n\n");

	// Read command line arguments -> short n_its (-n), float diff_const (-d);
	parse_cmd_args(argc, argv);

	// Read header 
	FILE *fp;
	fp = fopen("test_data/init_100_100", "r");

	int width, height;
	read_header(fp, &width, &height);

	printf("Width: %d\n", width);
	printf("Height: %d\n", height);

	fclose(fp);

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

	char *opencl_program_src;
	{
		FILE *clfp = fopen("./heat_calc.cl", "r");
		if ( clfp == NULL ) {
			fprintf(stderr, "could not load cl source code\n");
			return 1;
		}
		fseek(clfp, 0, SEEK_END);
		int clfsz = ftell(clfp);
		fseek(clfp, 0, SEEK_SET);
		opencl_program_src = (char*) malloc((clfsz+1)*sizeof(char));
		fread(opencl_program_src, sizeof(char), clfsz, clfp);
		opencl_program_src[clfsz] = 0;
		fclose(clfp);
	}

	cl_program program;
	size_t src_len = strlen(opencl_program_src);
	program = clCreateProgramWithSource(
			context, 1, (const char **) &opencl_program_src, (const size_t*) &src_len, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create program\n");
		return 1;
	}

	free(opencl_program_src);

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

	cl_kernel kernel_diffusion = clCreateKernel(program, "heat_diff", &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create kernel_diffusion\n");
		return 1;
	}

	cl_kernel kernel_average = clCreateKernel(program, "compute_average", &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create kernel_average\n");
		return 1;
	}
	
	// REMOVE WHEN NOT USING DUMMY MATRIX
	width = 3;
	height = 3;

	cl_mem input_buffer, output_buffer;
	input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY,
			width * height * sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create input buffer (for matrix_prev)\n");
		return 1;
	}
	output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
			width * height * sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create output buffer (for matrix_next)\n");
		return 1;
	}

	// Dummy 3 * 3 matrix with fixed value in the center and zero otherwise.
	float *matrix_prev = calloc(width * height, sizeof(float));
	if (!matrix_prev) {
        	fprintf(stderr, "Error allocating memory for matrix_prev.\n");
		return 1;
	}

	matrix_prev[4] = 1000000;

	const size_t global_sz[] = {width, height};

	// Compute heat diffusion
	
	float *matrix_next = malloc(width * height * sizeof(float));
	if (!matrix_next) {
		fprintf(stderr, "Error allocating memory for matrix_next.\n");
        	free(matrix_prev);
        	return 1;
	}

	clSetKernelArg(kernel_diffusion, 0, sizeof(cl_mem), &input_buffer);
	clSetKernelArg(kernel_diffusion, 1, sizeof(float), &diff_const);
	clSetKernelArg(kernel_diffusion, 2, sizeof(cl_mem), &output_buffer);
	clSetKernelArg(kernel_diffusion, 3, sizeof(int), &width);
	
	// Loop over the desired amount of iterations. --> Check if everything that is inside make sense to be inside or if it could be outside of the loop.
	
	for (size_t iteration = 0; iteration < n_its; iteration++){
		if ( clEnqueueWriteBuffer(command_queue,
					input_buffer, CL_TRUE, 0, width * height * sizeof(float), matrix_prev, 0, NULL, NULL)
				!= CL_SUCCESS ) {
			fprintf(stderr, "cannot enqueue write of buffer a\n");
			return 1;
		}
		
		// "for loop" in the kernel_diffusion
		if ( clEnqueueNDRangeKernel(command_queue, kernel_diffusion,
					2, NULL, (const size_t *) global_sz, NULL, 0, NULL, NULL)
				!= CL_SUCCESS ) { // Should make sure that the edges are actually 0.
						  // It turns out it's actually not, we are just taking the following cell in the vector so we are comparing the wrong cells. /R
			fprintf(stderr, "cannot enqueue kernel_diffusion\n");
			return 1;
		}

		if ( clEnqueueReadBuffer(command_queue,
					output_buffer, CL_TRUE, 0, width * height * sizeof(float), matrix_next, 0, NULL, NULL)
				!= CL_SUCCESS ) {
			fprintf(stderr, "cannot enqueue read of buffer c\n");
			return 1;
		}

		// Swap pointers; We can't just do matrix_prev = matrix_next because we would loose the reference to the original matrix_prev memory and loose the block.
		float *temp = matrix_prev;
        	matrix_prev = matrix_next;
        	matrix_next = temp;
	}
	

	if ( clFinish(command_queue) != CL_SUCCESS ) {
		fprintf(stderr, "cannot finish queue\n");
		return 1;
	}


	for (size_t jx = 0; jx < height; ++jx) {
		for (size_t ix=0; ix < width; ++ix)
			printf(" %5.f ", matrix_prev[jx * width + ix]); // The last iteration is stored in memory_prev because of the pointer swap.
		printf("\n");
	}

	// TODO 
	// - Compute average temperature and absolute difference with average --> Watch video on reduction.
	// - Compute diffusion from data file instead of dummy matrix

	free(matrix_prev);
	free(matrix_next);

	clReleaseMemObject(input_buffer);
	clReleaseMemObject(output_buffer);

	clReleaseProgram(program);
	clReleaseKernel(kernel_diffusion);
	clReleaseKernel(kernel_average);

	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);

	return 0;

}

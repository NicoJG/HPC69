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
	fp = fopen("init", "r");
	if (fp == NULL) {
		perror("file could not be opened.");
	}

	int width, height;
	read_header(fp, &width, &height);

	printf("Width: %d\n", width);
	printf("Height: %d\n", height);

	// We want to add a zero border around the entire matrix so we 
	// add +2 to both width and height
	width += 2;
	height +=2; 

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
		FILE *clfp = fopen("heat_calc.cl", "r");
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

	cl_kernel kernel_reduction = clCreateKernel(program, "compute_reduction", &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create kernel_reduction\n");
		return 1;
	}

	cl_kernel kernel_abs_diff = clCreateKernel(program, "heat_abs_diff", &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create kernel_heat_abs_diff\n");
		return 1;
	}

	// REMOVE WHEN NOT USING DUMMY MATRIX
	// width = 4;
	// height = 4;

	// EDIT TO MATCH FILE SIZE
	const int sz = width * height; // Number of cells; Needs to be divisible by 2 for the reduction to work
	const int global_redsz = 1024;
	const int local_redsz = 32;
	const int nmb_redgps = global_redsz / local_redsz;

	cl_mem input_buffer, output_buffer, output_buffer_sum, temp_buffer;
	input_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
			width * height * sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create input buffer (for matrix)\n");
		return 1;
	}
	output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
			width * height * sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create output buffer (for matrix_next)\n");
		return 1;
	}

	output_buffer_sum = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
			nmb_redgps * sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create output sum buffer (for computing the average)\n");
		return 1;
	}

	// Dummy 4 * 4 matrix with fixed value in the center and zero otherwise.
	// Now it should be (real) width * height instead!
	float *matrix = calloc(width * height, sizeof(float));
	if (!matrix) {
		fprintf(stderr, "Error allocating memory for matrix.\n");
		return 1;
	}


	read_and_initialise(fp, width, height, &matrix);
	fclose(fp);
	// matrix[5] = 10000;

	// changed this to 
	const size_t global_sz[] = {width - 2, height - 2};

	// Compute heat diffusion

	// Loop over the desired amount of iterations. --> Check if everything that is inside make sense to be inside or if it could be outside of the loop.

	// load the initial matrix into the input_buffer
	if ( clEnqueueWriteBuffer(command_queue,
				input_buffer, CL_TRUE, 0, width * height * sizeof(float), matrix, 0, NULL, NULL)
			!= CL_SUCCESS ) {
		fprintf(stderr, "cannot enqueue write of buffer a\n");
		return 1;
	}

	for (size_t iteration = 0; iteration < n_its; iteration++){
		clSetKernelArg(kernel_diffusion, 0, sizeof(cl_mem), &input_buffer);
		clSetKernelArg(kernel_diffusion, 1, sizeof(float), &diff_const);
		clSetKernelArg(kernel_diffusion, 2, sizeof(cl_mem), &output_buffer);
		clSetKernelArg(kernel_diffusion, 3, sizeof(int), &width);

		// "for loop" in the kernel_diffusion
		if ( clEnqueueNDRangeKernel(command_queue, kernel_diffusion,
					2, NULL, (const size_t *) global_sz, NULL, 0, NULL, NULL)
				!= CL_SUCCESS ) { // Should make sure that the edges are actually 0.
						  // It turns out it's actually not, we are just taking the following cell in the vector so we are comparing the wrong cells. /R
			fprintf(stderr, "cannot enqueue kernel_diffusion\n");
			return 1;
		}

		// The output of this iteration is the input of the next iteration
		// We can overwrite the input_buffer of this iteration as output in the next iteration
		temp_buffer = output_buffer;
		output_buffer = input_buffer;
		input_buffer = temp_buffer;
	}
	// rename it back to output_buffer after the forloop
	temp_buffer = output_buffer;
	output_buffer = input_buffer;
	input_buffer = temp_buffer;

	// read the last output matrix from the GPU
	if ( clEnqueueReadBuffer(command_queue,
				output_buffer, CL_TRUE, 0, width * height * sizeof(float), matrix, 0, NULL, NULL)
			!= CL_SUCCESS ) {
		fprintf(stderr, "cannot enqueue read of buffer output\n");
		return 1;
	}

	const cl_int sz_clint = (cl_int)sz;

	clSetKernelArg(kernel_reduction, 0, sizeof(cl_mem), &output_buffer);
	clSetKernelArg(kernel_reduction, 1, local_redsz*sizeof(float), NULL);
	clSetKernelArg(kernel_reduction, 2, sizeof(cl_int), &sz_clint);
	clSetKernelArg(kernel_reduction, 3, sizeof(cl_mem), &output_buffer_sum);

	size_t global_redsz_szt = (size_t) global_redsz;
	size_t local_redsz_szt = (size_t) local_redsz;
	if ( clEnqueueNDRangeKernel(command_queue,
				kernel_reduction, 1, NULL, (const size_t *) &global_redsz_szt, (const size_t *) &local_redsz_szt,
				0, NULL, NULL)
			!= CL_SUCCESS) {
		fprintf(stderr, "cannot enqueue kernel reduction\n");
		return 1;
	}

	float *sum = malloc(nmb_redgps*sizeof(float));
	if ( clEnqueueReadBuffer(command_queue,
				output_buffer_sum, CL_TRUE, 0, nmb_redgps*sizeof(float), sum, 0, NULL, NULL)
			!= CL_SUCCESS) {
		fprintf(stderr, "cannot enqueue read of buffer c sum\n");
		return 1;
	}

	float sum_total = 0.f;

	// We need to check if we have to divide the partial sums before if the total sum gets too big.
	for (size_t ix =0; ix < nmb_redgps; ++ix){
		sum_total += sum[ix];
		//printf("sum[%d] = %f\n", ix, sum[ix]);
	}
	sum_total /= ((width-2)*(height-2));

/*
	for (size_t jx = 0; jx < height; ++jx) {
		for (size_t ix=0; ix < width; ++ix)
			printf(" %5.2f ", matrix[jx * width + ix]); // The last iteration is stored in memory_prev because of the pointer swap.
		printf("\n");
	}
*/
	printf("Average is : %f\n", sum_total);

	clSetKernelArg(kernel_abs_diff, 0, sizeof(cl_mem), &output_buffer);
	clSetKernelArg(kernel_abs_diff, 1, sizeof(float), &sum_total);
	clSetKernelArg(kernel_abs_diff, 2, sizeof(int), &width);
	// TODO 
	// - Compute average temperature and absolute difference with average --> Watch video on reduction.
	// --> Average seems to work	
	// - Compute diffusion from data file instead of dummy matrix
	if ( clEnqueueNDRangeKernel(command_queue, kernel_abs_diff,
					2, NULL, (const size_t *) global_sz, NULL, 0, NULL, NULL)
				!= CL_SUCCESS ) { // Should make sure that the edges are actually 0.
						  // It turns out it's actually not, we are just taking the following cell in the vector so we are comparing the wrong cells. /R
			fprintf(stderr, "cannot enqueue kernel_abs_diff\n");
			return 1;
		}


	clSetKernelArg(kernel_reduction, 0, sizeof(cl_mem), &output_buffer);
	clSetKernelArg(kernel_reduction, 1, local_redsz*sizeof(float), NULL);
	clSetKernelArg(kernel_reduction, 2, sizeof(cl_int), &sz_clint);
	clSetKernelArg(kernel_reduction, 3, sizeof(cl_mem), &output_buffer_sum);

	if ( clEnqueueNDRangeKernel(command_queue,
				kernel_reduction, 1, NULL, (const size_t *) &global_redsz_szt, (const size_t *) &local_redsz_szt,
				0, NULL, NULL)
			!= CL_SUCCESS) {
		fprintf(stderr, "cannot enqueue kernel reduction\n");
		return 1;
	}

	//float *sum = malloc(nmb_redgps*sizeof(float));
	if ( clEnqueueReadBuffer(command_queue,
				output_buffer_sum, CL_TRUE, 0, nmb_redgps*sizeof(float), sum, 0, NULL, NULL)
			!= CL_SUCCESS) {
		fprintf(stderr, "cannot enqueue read of buffer c sum\n");
		return 1;
	}


	if ( clFinish(command_queue) != CL_SUCCESS ) {
		fprintf(stderr, "cannot finish queue\n");
		return 1;
	}

	//float sum_total = 0.f;

	// We need to check if we have to divide the partial sums before if the total sum gets too big.
	for (size_t ix =0; ix < nmb_redgps; ++ix){
		sum_total += sum[ix];
		//printf("sum[%d] = %f\n", ix, sum[ix]);
	}
	sum_total /= ((width-2)*(height-2));

	printf("Absolute average difference is : %f\n", sum_total);

	free(matrix);

	clReleaseMemObject(input_buffer);
	clReleaseMemObject(output_buffer);

	clReleaseProgram(program);
	clReleaseKernel(kernel_diffusion);
	clReleaseKernel(kernel_reduction);

	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);

	return 0;

}

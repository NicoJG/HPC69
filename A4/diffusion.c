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

	cl_kernel kernel = clCreateKernel(program, "heat_diff", &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create kernel\n");
		return 1;
	}

	const int width_a = 3;
	const int height_a = 3;

	cl_mem input_buffer_a, output_buffer_n;
	input_buffer_a = clCreateBuffer(context, CL_MEM_READ_ONLY,
			width_a*height_a * sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create buffer a\n");
		return 1;
	}
	output_buffer_n = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
			width_a*height_a * sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create buffer c\n");
		return 1;
	}

	// Dummy 3 * 3 matrix with fixed value in the center and zero otherwise.
	float *a = malloc(width_a * height_a * sizeof(float));
	memset(a, 0, width_a * height_a * sizeof(float));
	a[4] = 1000000;

	const size_t global_sz[] = {width_a, height_a};

	// Compute heat diffusion
	
	float *n = malloc(width_a * height_a * sizeof(float));

	for (size_t ix = 0; ix < n_its; ix++){
		if ( clEnqueueWriteBuffer(command_queue,
					input_buffer_a, CL_TRUE, 0, width_a * height_a * sizeof(float), a, 0, NULL, NULL)
				!= CL_SUCCESS ) {
			fprintf(stderr, "cannot enqueue write of buffer a\n");
			return 1;
		}
		clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer_a);
		clSetKernelArg(kernel, 1, sizeof(float), &diff_const);
		clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_buffer_n);
		clSetKernelArg(kernel, 3, sizeof(int), &width_a);
	
		if ( clEnqueueNDRangeKernel(command_queue, kernel,
					2, NULL, (const size_t *) global_sz, NULL, 0, NULL, NULL)
				!= CL_SUCCESS ) {
			fprintf(stderr, "cannot enqueue kernel\n");
			return 1;
		}

		if ( clEnqueueReadBuffer(command_queue,
					output_buffer_n, CL_TRUE, 0, width_a*height_a * sizeof(float), n, 0, NULL, NULL)
				!= CL_SUCCESS ) {
			fprintf(stderr, "cannot enqueue read of buffer c\n");
			return 1;
		}

		memcpy(a, n, width_a * height_a * sizeof(float));
	}

	if ( clFinish(command_queue) != CL_SUCCESS ) {
		fprintf(stderr, "cannot finish queue\n");
		return 1;
	}


	for (size_t jx = 0; jx < height_a; ++jx) {
		for (size_t ix=0; ix < width_a; ++ix)
			printf(" %5.f ", n[jx * width_a + ix]);
		printf("\n");
	}


	free(a);
	free(n);

	clReleaseMemObject(input_buffer_a);
	clReleaseMemObject(output_buffer_n);

	clReleaseProgram(program);
	clReleaseKernel(kernel);

	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);

	return 0;

}

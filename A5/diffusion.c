#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <x86intrin.h>

#include "global_vars.h"
#include "cmd_args.h"
#include "read_file.h"

static inline 
__m512 diffusion_formula_vectorized_16(const float *left, const float *center, const float *right) {
	// calculate result = center + diff_const * ((left+right+up+down)/4 - center)
	__m512 a_vec, b_vec, result_vec, tmp_vec;

	// add left and right (8 floats in each vector, start one row down)
	a_vec = _mm512_loadu_ps(left+1);
	b_vec = _mm512_loadu_ps(right+1);
	tmp_vec = _mm512_add_ps(a_vec, b_vec);

	// add up and down (8 floats in each vector)
	a_vec = _mm512_loadu_ps(center);
	b_vec = _mm512_loadu_ps(center+2);
	result_vec = _mm512_add_ps(a_vec, b_vec);

	// add the results (8 floats in each vector)
	result_vec = _mm512_add_ps(tmp_vec, result_vec);

	// divide by 4
	__m512 divisorVector = _mm512_set1_ps(4.f);
	result_vec = _mm512_div_ps(result_vec, divisorVector);

	// subtract center from result 
	a_vec = _mm512_loadu_ps(center+1);
	result_vec = _mm512_sub_ps(result_vec, a_vec);

	// multiply by diffusion constant
	__m512 constantVector = _mm512_set1_ps(diff_const);
	result_vec = _mm512_mul_ps(result_vec, constantVector);

	// add to center
	result_vec = _mm512_add_ps(result_vec, a_vec);

	return result_vec;
}

static inline 
__m256 diffusion_formula_vectorized_8(const float *left, const float *center, const float *right) {
	// calculate result = center + diff_const * ((left+right+up+down)/4 - center)
	__m256 a_vec, b_vec, result_vec, tmp_vec;

	// add left and right (8 floats in each vector, start one row down)
	a_vec = _mm256_loadu_ps(left+1);
	b_vec = _mm256_loadu_ps(right+1);
	tmp_vec = _mm256_add_ps(a_vec, b_vec);

	// add up and down (8 floats in each vector)
	a_vec = _mm256_loadu_ps(center);
	b_vec = _mm256_loadu_ps(center+2);
	result_vec = _mm256_add_ps(a_vec, b_vec);

	// add the results (8 floats in each vector)
	result_vec = _mm256_add_ps(tmp_vec, result_vec);

	// divide by 4
	__m256 divisorVector = _mm256_set1_ps(4.f);
	result_vec = _mm256_div_ps(result_vec, divisorVector);

	// subtract center from result 
	a_vec = _mm256_loadu_ps(center+1);
	result_vec = _mm256_sub_ps(result_vec, a_vec);

	// multiply by diffusion constant
	__m256 constantVector = _mm256_set1_ps(diff_const);
	result_vec = _mm256_mul_ps(result_vec, constantVector);

	// add to center
	result_vec = _mm256_add_ps(result_vec, a_vec);

	return result_vec;
}

static inline 
__m128 diffusion_formula_vectorized_4(const float *left, const float *center, const float *right) {
	// calculate result = center + diff_const * ((left+right+up+down)/4 - center)
	__m128 a_vec, b_vec, result_vec, tmp_vec;

	// add left and right (8 floats in each vector, start one row down)
	a_vec = _mm_loadu_ps(left+1);
	b_vec = _mm_loadu_ps(right+1);
	tmp_vec = _mm_add_ps(a_vec, b_vec);

	// add up and down (8 floats in each vector)
	a_vec = _mm_loadu_ps(center);
	b_vec = _mm_loadu_ps(center+2);
	result_vec = _mm_add_ps(a_vec, b_vec);

	// add the results (8 floats in each vector)
	result_vec = _mm_add_ps(tmp_vec, result_vec);

	// divide by 4
	__m128 divisorVector = _mm_set1_ps(4.f);
	result_vec = _mm_div_ps(result_vec, divisorVector);

	// subtract center from result 
	a_vec = _mm_loadu_ps(center+1);
	result_vec = _mm_sub_ps(result_vec, a_vec);

	// multiply by diffusion constant
	__m128 constantVector = _mm_set1_ps(diff_const);
	result_vec = _mm_mul_ps(result_vec, constantVector);

	// add to center
	result_vec = _mm_add_ps(result_vec, a_vec);

	return result_vec;
}

static inline
void perform_diffusion_vectorized(int rows, int full_width, int width, float *matrix_prev, float *matrix_next) {
	const int n_vec = 8;
	const int n_subrows = n_vec+2;
	// perform the diffusion on n_vec rows at the same time
	int i_row;
	float a[n_subrows], b[n_subrows], c[n_subrows];
	float *left=a, *center=b, *right=c, *tmp;
	float d[n_vec];
	float *result = d;
	for (i_row=1; i_row<(rows+1)-n_vec; i_row+=n_vec) {
		int i_col = 1;
		// load left and center only for first column
		for (int i_subrow=0; i_subrow<n_subrows; i_subrow++){
			left[i_subrow] = matrix_prev[(i_row-1+i_subrow)*full_width + (i_col - 1)];
			center[i_subrow] = matrix_prev[(i_row-1+i_subrow)*full_width + (i_col)];
		}

		for (; i_col<(width+1); i_col++) {
			// load right for each column
			for (int i_subrow=0; i_subrow<n_subrows; i_subrow++){
				right[i_subrow] = matrix_prev[(i_row-1+i_subrow)*full_width + (i_col + 1)];
			}
			
			__m256 result_vec = diffusion_formula_vectorized_8(left, center, right);
			_mm256_storeu_ps(result, result_vec);
			for (int i_subrow=0; i_subrow<n_vec; i_subrow++){
				matrix_next[(i_row+i_subrow)*full_width+i_col] = result[i_subrow];
			}
			
			tmp = left;
			left = center;
			center = right;
			right = tmp;
		}
	}
}

static inline 
__m256 diffusion_formula_vectorized_8_cols(__m256 left, __m256 center, __m256 right, __m256 up, __m256 down) {
	// calculate result = center + diff_const * ((left+right+up+down)/4 - center)
	__m256 result_vec, tmp_vec;

	// add left and right (8 floats in each vector, start one row down)
	tmp_vec = _mm256_add_ps(left, right);

	// add up and down (8 floats in each vector)
	result_vec = _mm256_add_ps(up, down);

	// add the results (8 floats in each vector)
	result_vec = _mm256_add_ps(tmp_vec, result_vec);

	// divide by 4
	__m256 divisorVector = _mm256_set1_ps(4.f);
	result_vec = _mm256_div_ps(result_vec, divisorVector);

	// subtract center from result 
	result_vec = _mm256_sub_ps(result_vec, center);

	// multiply by diffusion constant
	__m256 constantVector = _mm256_set1_ps(diff_const);
	result_vec = _mm256_mul_ps(result_vec, constantVector);

	// add to center
	result_vec = _mm256_add_ps(result_vec, center);

	return result_vec;
}

static inline
void perform_diffusion_vectorized_cols(int rows, int full_width, int width, float *matrix_prev, float *matrix_next) {
	const int n_vec = 8;
	// perform the diffusion on n_vec cols at the same time
	for (int i_row=1; i_row<(rows+1); i_row++) {
		int i_col = 1;
		for (; i_col<(width+1)-n_vec; i_col+=n_vec) {
			__m256 left = _mm256_loadu_ps(&matrix_prev[i_row*full_width+i_col-1]);
			__m256 center = _mm256_loadu_ps(&matrix_prev[i_row*full_width+i_col]);
			__m256 right = _mm256_loadu_ps(&matrix_prev[i_row*full_width+i_col+1]);
			__m256 up = _mm256_loadu_ps(&matrix_prev[(i_row-1)*full_width+i_col]);
			__m256 down = _mm256_loadu_ps(&matrix_prev[(i_row+1)*full_width+i_col]);

			__m256 result_vec = diffusion_formula_vectorized_8_cols(left, center, right, up, down);
			_mm256_storeu_ps(&matrix_next[i_row*full_width+i_col], result_vec);
		}
	}
}

static inline
void perform_diffusion(int rows, int full_width, int width, float *matrix_prev, float *matrix_next) {
	// remember the offset of 1 in each direction
	// do two rows at a time
	float left, center, right, left2, center2, right2;
	int i_row;
	for (i_row=1; i_row<(rows-1); i_row+=2) {
		int i = (i_row*full_width + 1);
		int i2 = i+full_width;
		int i_up = i-full_width;
		int i_down = i2+full_width;
		int end_of_row = i+width;
		left = matrix_prev[i-1];
		center = matrix_prev[i];
		left2 = matrix_prev[i2-1];
		center2 = matrix_prev[i2];
		for (; i<end_of_row; i++, i2++, i_up++, i_down++) {
			right = matrix_prev[i+1];
			float value = matrix_prev[i_up] 
						+ center2
						+ left 
						+ right;
			matrix_next[i] = center + diff_const * (value/4.f - center);

			right2 = matrix_prev[i2+1];
			float value2 = center 
						+ matrix_prev[i_down] 
						+ left2 
						+ right2;
			matrix_next[i2] = center2 + diff_const * (value2/4.f - center2);

			left = center;
			center = right;
			left2 = center2;
			center2 = right2;
		}
	}
	for (; i_row<(rows+1); i_row++) {
		int i = (i_row*full_width + 1);
		int i_up = i-full_width;
		int i_down = i+full_width;
		int end_of_row = i+width;
		left = matrix_prev[i-1];
		center = matrix_prev[i];
		for (; i<end_of_row; i++, i_up++, i_down++) {
			right = matrix_prev[i+1];
			float value = matrix_prev[i_up] 
						+ matrix_prev[i_down]
						+ left 
						+ right;
			matrix_next[i] = center + diff_const * (value/4.f - center);
			left = center;
			center = right;
		}
	}
}

int
main(
    int argc,
    char *argv[]
)
{
	// MPI general setup
	MPI_Init(&argc, &argv);
	int nmb_mpi_proc, mpi_rank; 
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nmb_mpi_proc);
	MPI_Status status;

	// Start program
	if (mpi_rank == 0)
		printf("\n ---------- Diffusion ----------\n\n");

	// Read command line arguments -> short n_its (-n), float diff_const (-d);
	// MPI_Init handled the MPI cmd_args and now it should be the same as in A4
	parse_cmd_args(argc, argv);
	if (mpi_rank == 0) {
		printf("Number of iterations : %d\n", n_its);
		printf("Diffusion constant is %f\n", diff_const);
	}
	
	int width, height, full_width, full_height;
	float *matrix;
	// read the input file only in the master process
	if (mpi_rank == 0) {
		FILE *fp;
		fp = fopen("init", "r");
		if (fp == NULL) {
			fprintf(stderr, "file could not be opened.");
		}
		read_header(fp, &width, &height);
		full_width = width+2;
		full_height = height+2;

		printf("Width: %d\n", width);
		printf("Height: %d\n", height);
		matrix = (float*)calloc(full_width*full_height, sizeof(float));
		if (!matrix) {
			fprintf(stderr, "Error allocating memory for matrix.\n");
			return 1;
		}

		read_and_initialise(fp, full_width, full_height, &matrix);
		fclose(fp);
	}

	// distribute information about the size to all processes
	MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&full_width, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&full_height, 1, MPI_INT, 0, MPI_COMM_WORLD);


	// prepare the scatterv command
	// prepare how many rows each process is responsible for
	// prepare the lengths and positions of the segments to send
	int rows, rowss[nmb_mpi_proc];
	int pos, poss[nmb_mpi_proc];
	int len, lens[nmb_mpi_proc];
	if (mpi_rank == 0)
		for (int i_rank = 0, pos = 0; i_rank < nmb_mpi_proc; i_rank++) {
			// determine how many rows each process is handling
			rowss[i_rank] = height/nmb_mpi_proc;
			if (i_rank == nmb_mpi_proc-1)
				// the last process also needs to process the remaining rows
				rowss[i_rank] += height % nmb_mpi_proc;
			// how many matrix elements does each process recieve?
			// each process needs also one row before and one row after its rows
			lens[i_rank] = (rowss[i_rank] + 2) * full_width;
			// set the position in the matrix 1D array
			poss[i_rank] = pos;
			pos += rowss[i_rank] * full_width;
		}

	// tell each process what it will recieve through Scatterv
	MPI_Scatter(rowss, 1, MPI_INT, &rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(poss, 1, MPI_INT, &pos, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(lens, 1, MPI_INT, &len, 1, MPI_INT, 0, MPI_COMM_WORLD);

	float *matrix_prev = (float*)calloc(len,sizeof(float));
	float *matrix_next = (float*)calloc(len,sizeof(float));

	// distribute the matrix sections to each process (with padding)
	MPI_Scatterv(matrix, lens, poss, MPI_FLOAT, matrix_prev, len, MPI_FLOAT, 0, MPI_COMM_WORLD);

	// now we don't need the full matrix anymore
	if (mpi_rank == 0) {
		free(matrix);
	}

	for (int i_iter = 0; i_iter < n_its; i_iter++) {
		// perform the diffusion on each matrix section
		// remember the offset of 1 in each direction
		//perform_diffusion(rows, full_width, width, matrix_prev, matrix_next);
		//perform_diffusion_vectorized(rows, full_width, width, matrix_prev, matrix_next);
		perform_diffusion_vectorized_cols(rows, full_width, width, matrix_prev, matrix_next);

		// switch matrix_next and matrix_prev
		float *tmp = matrix_next;
		matrix_next = matrix_prev;
		matrix_prev = tmp;

		// we only need to update the padding rows for each process
		float *send_row, *recieve_row;
		int send_to_rank, recieve_from_rank;

		// don't send and recieve stuff if there is only one process
		if (nmb_mpi_proc == 1)
			continue;

		// send the last row (non-padding row) 
		// and recieve the first row (padding row)
		send_row = matrix_prev + (len-2*full_width);
		recieve_row = matrix_prev;
		send_to_rank = mpi_rank + 1;
		recieve_from_rank = mpi_rank - 1;
		if (mpi_rank == 0) {
			MPI_Send(send_row, full_width, MPI_FLOAT, send_to_rank, 0, MPI_COMM_WORLD);
		} else if (mpi_rank == nmb_mpi_proc - 1) {
			MPI_Recv(recieve_row, full_width, MPI_FLOAT, recieve_from_rank, 0, MPI_COMM_WORLD, &status);
		} else {
			MPI_Sendrecv(send_row,full_width,MPI_FLOAT,send_to_rank,0,
						recieve_row,full_width,MPI_FLOAT,recieve_from_rank,0,
						MPI_COMM_WORLD, &status);
		}

		// send the first row (non-padding row) 
		// and recieve the last row (padding row)
		send_row = matrix_prev + full_width;
		recieve_row = matrix_prev + (len-full_width);
		send_to_rank = mpi_rank - 1;
		recieve_from_rank = mpi_rank + 1;
		if (mpi_rank == nmb_mpi_proc - 1) {
			MPI_Send(send_row, full_width, MPI_FLOAT, send_to_rank, 0, MPI_COMM_WORLD);
		} else if (mpi_rank == 0) {
			MPI_Recv(recieve_row, full_width, MPI_FLOAT, recieve_from_rank, 0, MPI_COMM_WORLD, &status);
		} else {
			MPI_Sendrecv(send_row,full_width,MPI_FLOAT,send_to_rank,0,
						recieve_row,full_width,MPI_FLOAT,recieve_from_rank,0,
						MPI_COMM_WORLD, &status);
		}
	}
	

	// calculate the sum of the matrix section in each process
	double sum = 0;
	for (int iy = 1; iy < (rows+1); iy++) {
		for (int ix = 1; ix < (width+1); ix++) {
			sum += matrix_prev[iy*full_width+ix];
		}
	}

	// get the sum of all matrix section sums
	double avg_total = 0;
	MPI_Reduce(&sum, &avg_total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	// calculate the average in the master process
	if (mpi_rank == 0) {
		avg_total /= width*height;
	}

	// send the average to each process
	MPI_Bcast(&avg_total, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// calculate the sum of the differences to the average
	sum = 0;
	for (int iy = 1; iy < (rows+1); iy++) {
		for (int ix = 1; ix < (width+1); ix++) {
			sum += fabs(matrix_prev[iy*full_width+ix] - avg_total);
		}
	}

	// get the sum of all average differences
	double avg_diff_total = 0;
	MPI_Reduce(&sum, &avg_diff_total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	// calculate the average in the master process
	if (mpi_rank == 0) {
		avg_diff_total /= width*height;
		printf("Average is : %f\n", avg_total);
		printf("Absolute average difference is : %f\n", avg_diff_total);
	}

	free(matrix_prev);
	free(matrix_next);
    MPI_Finalize();
	return 0;
}

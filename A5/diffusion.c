#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

#include "global_vars.h"
#include "cmd_args.h"
#include "read_file.h"


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

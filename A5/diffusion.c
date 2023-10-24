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

		/* print the matrix
		for (int iy = 0; iy < height+2; iy++) {
			for (int ix = 0; ix < full_width; ix++) {
				printf("%.2e ",matrix[iy*full_width+ix]);
			}
			printf("\n");
		}
		*/
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

	// the gatherv command should not send back the padding
	// we need new positions and lengths
	int gather_pos, gather_poss[nmb_mpi_proc];
	int gather_len, gather_lens[nmb_mpi_proc];
	if (mpi_rank == 0)
		for (int i_rank = 0, pos = full_width; i_rank < nmb_mpi_proc; i_rank++) {
			// how many matrix elements does each process recieve?
			// each process needs also one row before and one row after its rows
			gather_lens[i_rank] = rowss[i_rank] * full_width;
			// set the position in the matrix 1D array
			gather_poss[i_rank] = pos;
			pos += rowss[i_rank] * full_width;
		}

	// tell each process what it will recieve through Scatterv
	MPI_Scatter(rowss, 1, MPI_INT, &rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(poss, 1, MPI_INT, &pos, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(lens, 1, MPI_INT, &len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(gather_poss, 1, MPI_INT, &gather_pos, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(gather_lens, 1, MPI_INT, &gather_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	printf("%i %i %i %i\n", mpi_rank,rows, pos, pos+len);
	printf("%i %i %i %i\n", mpi_rank,rows, gather_pos, gather_pos+gather_len);

	float *matrix_prev = (float*)malloc(sizeof(float)*len);
	float *matrix_next = (float*)malloc(sizeof(float)*len);

	for (int i_iter = 0; i_iter < n_its; i_iter++) {
		// distribute the matrix sections to each process (with padding)
		MPI_Scatterv(matrix, lens, poss, MPI_FLOAT, matrix_prev, len, MPI_FLOAT, 0, MPI_COMM_WORLD);

		// copy the matrix to fix the padding for Gatherv
		for (int i=0; i<len; i++) {
			matrix_next[i] = matrix_prev[i];
		}

		// perform the diffusion on each matrix section
		// remember the offset of 1 in each direction
		//printf("%e\n", matrix_next[width+50+i_iter]);
		for (int iy = 1; iy < (rows+1); iy++) {
			for (int ix = 1; ix < (width+1); ix++) {
				int idx = iy*full_width+ix;
				float value = matrix_prev[(iy - 1)*full_width + ix]
							+ matrix_prev[(iy + 1)*full_width + ix]
							+ matrix_prev[iy*full_width + ix-1]
							+ matrix_prev[iy*full_width + ix+1];
				matrix_next[idx] = matrix_prev[idx] + diff_const * (value/4 - matrix_prev[idx]);
				//matrix_next[idx] = matrix_prev[idx];
			}
		}

		// send the diffused matrix sections back to the master process (without padding)
		MPI_Gatherv(matrix_next+full_width, gather_len, MPI_FLOAT, matrix, gather_lens, gather_poss, MPI_FLOAT, 0, MPI_COMM_WORLD);

		/* // print matrix segment
		printf("\n\n\n\niter %i, mpi_rank %i:\n", i_iter, mpi_rank);
		for (int iy = 0; iy < (rows+2); iy++) {
				printf("row %i:\n", iy);
			for (int ix = 0; ix < (width+2); ix++) {
				printf("%.2e ",matrix_next[iy*full_width+ix]);
			}
		} */
				

		/* // print the matrix 
		if (mpi_rank == 0) {
			printf("\n\n\n\niter %i:\n", i_iter);
			for (int iy = 0; iy < height+2; iy++) {
				printf("row %i:\n", iy);
				for (int ix = 0; ix < full_width; ix++) {
					printf("%.2e ",matrix[iy*full_width+ix]);
				}
				printf("\n");
			}
		} */
	}
	

	// calculate the sum of the matrix section in each process
	double sum = 0;
	for (int iy = 1; iy < (rows+1); iy++) {
		for (int ix = 1; ix < (width+1); ix++) {
			sum += matrix_next[iy*full_width+ix];
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
			sum += fabs(matrix_next[iy*full_width+ix] - avg_total);
		}
	}

	// get the sum of all average differences
	double avg_diff_total = 0;
	MPI_Reduce(&sum, &avg_diff_total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	// calculate the average in the master process
	if (mpi_rank == 0) {
		avg_total /= width*height;
		printf("Average is : %f\n", avg_total);
		printf("Absolute average difference is : %f\n", avg_diff_total);
	}

	free(matrix_prev);
	free(matrix_next);
	if (mpi_rank == 0) {
		free(matrix);
	}
    MPI_Finalize();
	return 0;
}

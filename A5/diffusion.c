#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
// needs to be changed to handle mpirun cmd_args probably
#include "cmd_args.h"
#include "read_file.h"

// i'm not sure at all where the best place to do the iterations is, so maybe this code needs to be
// reworked. one possibility is to set it around the send and receive parts in the root mpi, but
// maybe that would cause a deadlock, i'm really not sure.

int
main(
    int argc,
    char *argv[]
)
{
	// Start program
	printf("\n ---------- Diffusion ----------\n\n");

	// placeholder for new function?
	parse_cmd_args(argc, argv);
	int width, height;
	MPI_Status status;
	// maybe we should do this inside the root mpi? start
	FILE *fp;
	fp = fopen("test_data/init_100_100", "r");
	if (fp == NULL) {
		fprintf(stderr, "file could not be opened.");
	}
	read_header(fp, &width, &height);

	printf("Width: %d\n", width);
	printf("Height: %d\n", height);
	float *matrix = calloc(width * height, sizeof(float));
	if (!matrix) {
		fprintf(stderr, "Error allocating memory for matrix.\n");
		return 1;
	}


	read_and_initialise(fp, width, height, &matrix);
	fclose(fp);
	// end
	int nmb_mpi_proc, mpi_rank, calc_proc_count, source, dest, rows, offset;
	MPI_Init(&argc, &argv);
    	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nmb_mpi_proc);
	calc_proc_count = nmb_mpi_proc - 1;
	// this needs to be altered to work when the number of rows is not divisible
	rows = height/calc_proc_count;
	// this could also go in the root mpi i think, start
	float *matrix_calc = calloc(width*height, sizeof(float));
	if (!matrix_calc) {
		fprintf(stderr, "Error allocating memory for matrix.\n");
		return 1;
	}
	// end
	if (mpi_rank == 0) {
		// the "official" rows start on the second row
    		offset = 1;
		for (dest=1; dest <= calc_proc_count; dest++) {
      			MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      			MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
			// send rows above and below allocated chunk, but they will not be calculated
      			MPI_Send(&matrix[(offset-1)*width], (rows+2)*width, MPI_FLOAT,dest,1, MPI_COMM_WORLD);
      			offset = offset + rows;
		}
		for (int i = 1; i <= calc_proc_count; i++) {
      			source = i;
      			MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      			MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
		        MPI_Recv(&matrix_calc[offset*width], rows*width, MPI_FLOAT, source, 2, MPI_COMM_WORLD, &status);
    		}

	}
	 if (mpi_rank > 0) {
    	 	source = 0;
    		MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
    		MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&matrix, rows*width, MPI_FLOAT, source, 1, MPI_COMM_WORLD, &status);
		// maybe this allocation is unnecessary, we could probably use the topmost rows of matrix_calc
		// in every process but it helped me think to have it here
		float *matrix_result = calloc(rows*width, sizeof(float));
		if (!matrix_result) {
			fprintf(stderr, "Error allocating memory for matrix.\n");
			return 1;
		}
		// my brain gave up with these indices, but basically we want the result matrix to be without the
		// additional top and bottom rows we send but still have the zero columns (maybe, could probably
		// be changed in the root mpi to be less confusing
    		for (int ix = 1; ix < width; ix++) {
      			for (int iy = 1; iy < rows; iy++) {
				matrix_result[(iy-1)*width + ix] = matrix[(iy -1)*width + ix] + matrix[(iy+1)*width + ix] + matrix[iy*width + ix-1] + matrix[iy*width+ix+1];
      			}
    		}
    		MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        	MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        	MPI_Send(&matrix_result, rows*width, MPI_FLOAT, 0, 2, MPI_COMM_WORLD);
       }

       MPI_Finalize();
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#include "global_vars.h"
#include "cmd_args.h"
#include "computations.h"
#include "write_ppm.h"

// Remember that we use global variables (in global_vars.h)
// This way we don't need to send some vars to the thread functions
// Those variables can be accessed everywhere
// but make sure not to write to those vars at the same time (in different threads)

typedef struct {
	int ix_start;
	int ix_step;
	int i_thrd;
	mtx_t *mtx;
	cnd_t *cnd;
	int *status;
} thrd_info_compute_t;

typedef struct {
	FILE *file_attractors;
	FILE *file_convergence;
	mtx_t *mtx;
	cnd_t *cnd;
	int *status;
} thrd_info_write_t;

int compute_thread(void *args) {
	// Unpack arguments
	const thrd_info_compute_t *thrd_info = (thrd_info_compute_t*) args;
	const int ix_start = thrd_info->ix_start;
	const int ix_step = thrd_info->ix_step;
	const int i_thrd = thrd_info->i_thrd;
	mtx_t *mtx = thrd_info->mtx;
	cnd_t *cnd = thrd_info->cnd;
	int *status = thrd_info->status;

	// Iterate through the rows that are assigned to this thread
	for (int ix = ix_start; ix < image_size; ix += ix_step) {
		// Allocate memory for row ix
		short *root_idxs_row = (short *) malloc(sizeof(short) * image_size);
		short *n_its_row = (short *) malloc(sizeof(short) * image_size);

		// Do the Newton iteration on each entry of row ix
		for (int jx = 0; jx < image_size; ++jx) {
			// Get the position that (ix,jx) corresponds to
			double complex x0 = get_x0(ix, jx); 

			// perform newton iteration and save the result in the jx item
			newton_iteration(x0, root_idxs_row + jx, n_its_row + jx);
		}

		// Lock so we don't read and write to matrices at the same time
		mtx_lock(mtx);
		root_idxs[ix] = root_idxs_row;
		n_its[ix] = n_its_row;
		// update which row this thread is working on next
		status[i_thrd] = ix + ix_step; 
		mtx_unlock(mtx);
		cnd_signal(cnd);
	}

	return 0;
}

int write_thread(void *args) {
	// Unpack arguments
	const thrd_info_write_t *thrd_info = (thrd_info_write_t*) args;
	FILE *file_attractors = thrd_info->file_attractors;
	FILE *file_convergence = thrd_info->file_convergence;
	mtx_t *mtx = thrd_info->mtx;
	cnd_t *cnd = thrd_info->cnd;
	int *status = thrd_info->status;

	// No incrementation in this loop
	for (int ix = 0, ibnd; ix < image_size; ) {

		// Check if new row is available
		for (mtx_lock(mtx); ; ) {
			
			// Get the minimum of the status values (I think this ensures rows are not written out of order)
			// I agree, kinda, I think it's to make sure all the rows between ix and ibnd are actually done
			// before writing them to the file and not just the random values from initialization
			ibnd = image_size;
			for (int i_thrd = 0; i_thrd < n_threads; ++i_thrd) {
				if (ibnd > status[i_thrd]) {
					ibnd = status[i_thrd];
				}
			}

			if (ibnd <= ix) {
				cnd_wait(cnd, mtx);

			} else {
				mtx_unlock(mtx);
				break;
			}
		}

		// SHOULD WRITE THE IMAGES HERE
		// iterate through the rows
		for ( ; ix < ibnd; ++ix) {
			write_attractors_row(file_attractors, root_idxs[ix]);
			write_convergence_row(file_convergence, n_its[ix]);

			// Now we can free the rows of the arrays
			free(root_idxs[ix]);
			free(n_its[ix]);
		}	
		fprintf(stderr, "Wrote until row %i/%i\n", ibnd, image_size);
	}

	return 0;
}


int main(int argc, char *argv[]){

	// Start program
	printf("\n---------- Newton ----------\n\n");

	// read command line arguments -> nthreads, image_size, order
	parse_cmd_args(argc, argv);

	// LAYOUT:
    // setup arrays -> attractors, convergences, roots
	// Allocate double pointers to the rows of the two images but allocate
	// the row entries in the threads as we go
	// Global variables:
	root_idxs = (short **) malloc(sizeof(short *) * image_size);
	n_its = (short **) malloc(sizeof(short *) * image_size);

	// Prepare a list of the roots
	roots = (double complex *) malloc(sizeof(double complex) * order);
	for (int i_root = 0; i_root < order; i_root++) {
		roots[i_root] = get_root_by_index(i_root);
		printf("root %d: %f + %fi\n", i_root, creal(roots[i_root]), cimag(roots[i_root]));
	}

	// Open files for writing
	char filename[50];
	sprintf(filename, "newton_attractors_x%d.ppm", order);
	FILE *file_attractors = fopen(filename, "w");

	if (file_attractors == NULL){
		printf("error opening file\n");
		return -1;
	}
	
	sprintf(filename, "newton_convergence_x%d.ppm", order);
	FILE *file_convergence = fopen(filename, "w");

	if (file_convergence == NULL){
		printf("error opening file\n");
		return -1;
	}

	// write the PPM headers
	write_ppm_header(file_attractors);
	write_ppm_header(file_convergence);

    // iterate over pixels
    // -> get position (x0)
    // -> newton iteration
    // --> check if x is converged (x - root[i]) <= CONV_DIST
    // --> check if i_newton > MAX_NEWTON_ITERS
    // --> calculate newton iteration

    // write ppm files


	// Initialise all variables needed for the threads

	thrd_t thrds_compute[n_threads];
	thrd_info_compute_t thrds_info_compute[n_threads];

	thrd_t thrd_write;
	thrd_info_write_t thrd_info_write;

	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);

	cnd_t cnd;
	cnd_init(&cnd);

	int status[n_threads];

	// Start the computation threads
	for (int i_thrd = 0; i_thrd < n_threads; ++i_thrd) {
		thrds_info_compute[i_thrd].ix_start = i_thrd;
		thrds_info_compute[i_thrd].ix_step = n_threads;
		thrds_info_compute[i_thrd].i_thrd = i_thrd;
		thrds_info_compute[i_thrd].mtx = &mtx;
		thrds_info_compute[i_thrd].cnd = &cnd;
		thrds_info_compute[i_thrd].status = status;
		status[i_thrd] = 0;

		int r = thrd_create(
					&thrds_compute[i_thrd], 
					compute_thread, 
					(void*) &thrds_info_compute[i_thrd]
				);
    	if ( r != thrd_success ) {
			fprintf(stderr, "failed to create thread\n");
			exit(1);
    	}

		// Martin adds "thrd_detach" here but I'm not entirely sure what it does!
		// Isak

		thrd_detach(thrds_compute[i_thrd]);
	}

	// Start the writing thread
	{
		thrd_info_write.file_attractors = file_attractors;
		thrd_info_write.file_convergence = file_convergence;
		thrd_info_write.mtx = &mtx;
		thrd_info_write.cnd = &cnd;
		thrd_info_write.status = status;

		int r = thrd_create(
					&thrd_write, 
					write_thread, 
					(void*) (&thrd_info_write)
				);
		if ( r != thrd_success ) {
			fprintf(stderr, "failed to create thread\n");
			exit(1);
		}
  	}

	// wait until the writing thread has finished
	{
		int r;
		thrd_join(thrd_write, &r);
	}

	// free variables
	free(root_idxs);
	free(n_its);
	free(roots);

	// Close files
	fclose(file_attractors);
	fclose(file_convergence);

	return 0;
} 

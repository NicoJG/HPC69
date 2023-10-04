#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#include "global_vars.h"
#include "cmd_args.h"
#include "computations.h"

// Remember that we use global variables (in global_vars.h)
// This way we don't need to send some vars to the thread functions
// Those variables can be accessed everywhere
// but make sure not to write to those vars at the same time (in different threads)


// I don't know why we need this padded int /Nico
typedef struct {
	int val;
	char pad[60]; 
} int_padded;

typedef struct {
	int ix_start;
	int ix_step;
	int i_thrd;
	mtx_t *mtx;
	cnd_t *cnd;
	int_padded *status;
} thrd_info_compute_t;

typedef struct {
	mtx_t *mtx;
	cnd_t *cnd;
	int_padded *status;
} thrd_info_write_t;

int compute_thread(void *args) {
	// Unpack arguments
	const thrd_info_compute_t *thrd_info = (thrd_info_compute_t*) args;
	const int ix_start = thrd_info->ix_start;
	const int ix_step = thrd_info->ix_step;
	const int i_thrd = thrd_info->i_thrd;
	mtx_t *mtx = thrd_info->mtx;
	cnd_t *cnd = thrd_info->cnd;
	int_padded *status = thrd_info->status;

	// Initialise needed variables
	double complex x;

	// Iterate through the rows that are assigned to this thread
	for (int ix = ix_start; ix < image_size; ix += ix_step) {
		// Allocate memory for row ix
		short *root_idxs_row = (short *) malloc(sizeof(short) * image_size);
		short *n_its_row = (short *) malloc(sizeof(short) * image_size);

		// Do the Newton iteration on each entry of row ix
		for (int jx = 0; jx < image_size; ++jx) {
			// Get the position that (ix,jx) corresponds to
			x = get_x0(ix, jx); // Don't use this yet but I guess we will eventually!

			// perform newton iteration

			// Save which root the Newton method converged to
			root_idxs_row[jx] = jx; // DUMMY
			// Save how many iterations the Newton method took
			n_its_row[jx] = ix; // DUMMY
		}

		// Lock so we don't read and write to matrices at the same time
		mtx_lock(mtx);
		root_idxs[ix] = root_idxs_row;
		n_its[ix] = n_its_row;
		// update which row this thread is working on next
		status[i_thrd].val = ix + ix_step; 
		mtx_unlock(mtx);
		cnd_signal(cnd);
	}

	return 0;
}

int write_thread(void *args) {
	// Unpack arguments
	const thrd_info_write_t *thrd_info = (thrd_info_write_t*) args;
	mtx_t *mtx = thrd_info->mtx;
	cnd_t *cnd = thrd_info->cnd;
	int_padded *status = thrd_info->status;

	// No incrementation in this loop
	for (int ix = 0, ibnd; ix < image_size; ) {

		// Check if new row is available
		for (mtx_lock(mtx); ; ) {
			
			// Get the minimum of the status values
			ibnd = image_size;
			for (int i_thrd = 0; i_thrd < n_threads; ++i_thrd) {
				if (ibnd > status[i_thrd].val) {
					ibnd = status[i_thrd].val;
				}
			}

			if (ibnd <= ix) {
				cnd_wait(cnd, mtx);

			} else {
				mtx_unlock(mtx);
				break;
			}
		}

		fprintf(stderr, "checking until %i\n", ibnd);

		// SHOULD WRITE THE IMAGES HERE
		for ( ; ix < ibnd; ++ix) {
			for (int jx = 0; jx < image_size; ++jx) {
				printf("%d, ", n_its[ix][jx]);
			}
			printf("\n");

			// Now we can free the rows of the arrays
			free(root_idxs[ix]);
			free(n_its[ix]);
		}	

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

    // iterate over pixels
    // -> get position (x0)
    // -> newton iteration
    // --> check if x is converged (x - root[i]) <= CONV_DIST
    // --> check if i_newton > MAX_NEWTON_ITERS
    // --> calculate newton iteration

    // write ppm files

	// Allocate double pointers to the rows of the two images but allocate
	// the entries in the threads as we go
	// Global variables:
	root_idxs = (short **) malloc(sizeof(short *) * image_size);
	n_its = (short **) malloc(sizeof(short *) * image_size);

	// Initialise all variables needed for the threads

	thrd_t thrds_compute[n_threads];
	thrd_info_compute_t thrds_info_compute[n_threads];

	thrd_t thrd_write;
	thrd_info_write_t thrd_info_write;

	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);

	cnd_t cnd;
	cnd_init(&cnd);

	int_padded status[n_threads];

	// Start the computation threads
	for (int i_thrd = 0; i_thrd < n_threads; ++i_thrd) {
		thrds_info_compute[i_thrd].ix_start = i_thrd;
		thrds_info_compute[i_thrd].ix_step = n_threads;
		thrds_info_compute[i_thrd].i_thrd = i_thrd;
		thrds_info_compute[i_thrd].mtx = &mtx;
		thrds_info_compute[i_thrd].cnd = &cnd;
		thrds_info_compute[i_thrd].status = status;
		status[i_thrd].val = 0;

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

	return 0;
} 



/*
I think a good way of designing the program is to let a couple of threads work together on one line at a time.
By doing this we will finish line-by-line and be able to write the lines as we go along with the computations, 
using a different thread. I think this is what's being done in the video on multi-stage processing.

// Isak

*/
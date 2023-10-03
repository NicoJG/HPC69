#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#include "global_vars.h"
#include "cmd_args.h"
#include "computations.h"


typedef struct {
	int val;
	char pad[60]; 
} int_padded;

typedef struct {
	float **which_root;
	short **n_its;
	int ib;
	int istep;
	int image_size;
	int tx;
	mtx_t *mtx;
	cnd_t *cnd;
	int_padded *status;
} thrd_info_t;

typedef struct {
	float **which_root;
	short **n_its;
	int image_size;
	int n_threads;
	mtx_t *mtx;
	cnd_t *cnd;
	int_padded *status;
} thrd_info_check_t;

int 
compute_thread(
	void *args
) {
	
	// Unpack arguments
	const thrd_info_t *thrd_info = (thrd_info_t*) args;
	float **which_root = thrd_info->which_root;
	short **n_its = thrd_info->n_its;
	const int ib = thrd_info->ib;
	const int istep = thrd_info->istep;
	const int image_size = thrd_info->image_size;
	const int tx = thrd_info->tx;
	mtx_t *mtx = thrd_info->mtx;
	cnd_t *cnd = thrd_info->cnd;
	int_padded *status = thrd_info->status;

	// Allocate the rows of which_root and n_its directly here in the computation thread
	for (int ix = ib; ix < image_size; ix += istep) {
		float *which_root_entrs = (float *) malloc(sizeof(float) * image_size);
		short *no_ints_entrs = (short *) malloc(sizeof(short) * image_size);

		// Do the computations here
		for (int jx = 0; jx < image_size; ++jx) {
			which_root_entrs[jx] = jx; // DUMMY
			no_ints_entrs[jx] = 2 * jx; // DUMMY
		}

		// Lock so we don't read and write to matrices at the same time
		mtx_lock(mtx);
		which_root[ix] = which_root_entrs;
		n_its[ix] = no_ints_entrs;
		status[tx].val = ix + istep;
		mtx_unlock(mtx);
		cnd_signal(cnd);
	}

	return 0;
}

int 
check_compute_thread(
	void *args
) {
	// Unpack arguments
	const thrd_info_check_t *thrd_info = (thrd_info_check_t*) args;
	float **which_root = thrd_info->which_root;
	short **n_its = thrd_info->n_its;
	const int image_size = thrd_info->image_size;
	const int n_threads = thrd_info->n_threads;
	mtx_t *mtx = thrd_info->mtx;
	cnd_t *cnd = thrd_info->cnd;
	int_padded *status = thrd_info->status;

	// No incrementation in this loop
	for (int ix = 0, ibnd; ix < image_size; ) {

		// Check if new line is available
		for (mtx_lock(mtx); ; ) {
			
			// Extract status variables
			ibnd = image_size;
			for (int tx = 0; tx < n_threads; ++tx) {
				if (ibnd > status[tx].val) {
					ibnd = status[tx].val;
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

		for ( ; ix < ibnd; ++ix) {
			for (int jx = 0; jx < image_size; ++jx) {
				printf("%d, ", n_its[ix][jx]);
			}
			printf("\n");

			// Now we can free the rows of the arrays
			free(which_root[ix]);
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
    // setup arrays -> attractors, convergences, roots

    // iterate over pixels
    // -> get position (x0)
    // -> newton iteration
    // --> check if x is converged (x - root[i]) <= CONV_DIST
    // --> check if i_newton > MAX_NEWTON_ITERS
    // --> calculate newton iteration

    // write ppm files

	// Allocate double pointers to the rows of the two images but save the 
	// the allocations of entries to a different thread
	float **which_root = (float **) malloc(sizeof(float *) * image_size);
	short **n_its = (short **) malloc(sizeof(short *) * image_size);

	// Initialise all variables needed for the threads
	thrd_t thrds[n_threads];
	thrd_info_t thrds_info[n_threads];

	thrd_t thrd_check;
	thrd_info_check_t thrd_info_check;

	mtx_t mtx;
	mtx_init(&mtx, mtx_plain);

	cnd_t cnd;
	cnd_init(&cnd);

	int_padded status[n_threads];

	for (int tx = 0; tx < n_threads; ++tx) {
		thrds_info[tx].which_root = which_root;
		thrds_info[tx].n_its = n_its;
		thrds_info[tx].ib = tx;
		thrds_info[tx].istep = n_threads;
		thrds_info[tx].image_size = image_size;
		thrds_info[tx].tx = tx;
		thrds_info[tx].mtx = &mtx;
		thrds_info[tx].cnd = &cnd;
		thrds_info[tx].status = status;
		status[tx].val = 0;

		int r = thrd_create(&thrds[tx], compute_thread, (void*) &thrds_info[tx]);
    	if ( r != thrd_success ) {
			fprintf(stderr, "failed to create thread\n");
			exit(1);
    	}

		// Martin adds "thrd_detach" here but I'm not entirely sure what it does!
		// Isak

		thrd_detach(thrds[tx]);
	}

	// Check status
	{
		thrd_info_check.which_root= which_root;
		thrd_info_check.n_its = n_its;
		thrd_info_check.image_size = image_size;
		thrd_info_check.n_threads = n_threads;
		thrd_info_check.mtx = &mtx;
		thrd_info_check.cnd = &cnd;
		thrd_info_check.status = status;

		int r = thrd_create(&thrd_check, check_compute_thread, (void*) (&thrd_info_check));
		if ( r != thrd_success ) {
			fprintf(stderr, "failed to create thread\n");
			exit(1);
		}
  	}

  {
    int r;
    thrd_join(thrd_check, &r);
  }

	// free variables
	free(which_root);
	free(n_its);

	return 0;
} 



/*
I think a good way of designing the program is to let a couple of threads work together on one line at a time.
By doing this we will finish line-by-line and be able to write the lines as we go along with the computations, 
using a different thread. I think this is what's being done in the video on multi-stage processing.

// Isak

*/
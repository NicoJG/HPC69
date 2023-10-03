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
	short **no_its;
	int ib;
	int istep;
	int sz;
	int tx;
	mtx_t *mtx;
	cnd_t *cnd;
	int_padded *status;
} thrd_info_t;

typedef struct {
	float **which_root;
	short **no_its;
	int sz;
	int nthreads;
	mtx_t *mtx;
	cnd_t *cnd;
	int_padded *status;
} thrd_info_check_t;

int 
compute_thread(
	void *args
) {
	
}


int main(int argc, char *argv[]){
	
	//Default values
	int n_threads = DEFAULT_THREADS;
	int image_sz = DEFAULT_IMAGE_SIZE;
	int order = DEFAULT_ORDER;
	int opt, val;

	printf("\n---------- Newton ----------\n\n");

	// read command line arguments -> nthreads, image_sz, order
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
	float **which_root = (float **) malloc(sizeof(float *) * image_sz);
	short **no_its = (short **) malloc(sizeof(short *) * image_sz);

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
		thrds_info[tx].no_its = no_its;
		thrds_info[tx].ib = tx;
		thrds_info[tx].istep = n_threads;
		thrds_info[tx].sz = image_sz;
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

		// thrd_detach(thrds[tx]);
	}

	// free variables
	free(which_root);
	free(no_its);

	return 0;
} 



/*
I think a good way of designing the program is to let a couple of threads work together on one line at a time.
By doing this we will finish line-by-line and be able to write the lines as we go along with the computations, 
using a different thread. I think this is what's being done in the video on multi-stage processing.

// Isak

*/
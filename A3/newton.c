#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#include "global_vars.h"
#include "cmd_args.h"
#include "computations.h"


int main(int argc, char *argv[]){
	
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

    // free variables

	return 0;
} 

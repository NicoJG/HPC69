#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

#include "global_vars.h"

static inline
double complex get_x0(short i_row, short i_col) {
    double complex x0 = 0;
    x0 +=    X_MIN_RE + (X_MAX_RE-X_MIN_RE) * i_row / image_size;
    x0 += I*(X_MIN_IM + (X_MAX_IM-X_MIN_IM) * i_col / image_size);
    return x0;
}

static inline
double complex get_root_by_index(short i_root) {
    assert(0 <= i_root && i_root < order);
    return cexp(I*(2 * PI * i_root / order));
}

static inline
double complex newton_iteration_step(double complex x_prev, int degree) {

	// Analytical expression is 	
	/*
	           ((d - 1) * x^d + 1) 
	x_next = -----------------------
	               d * x^(d-1)
	*/
	double complex x_next;
	//double complex x_d = 1;
	
	/*
	for (size_t ix = 0; ix < degree; ix++){
		x_d *= x_prev;
	}
	x_next = ( (degree - 1) * x_d + 1) / (degree * x_d / x_prev);

	*/

	// TODO: "Inserting the Newton iteration step naively, you obtain x - (x^d - 1)/(d*x^(d-1)). How can you simplify it?""
	
	switch (degree) {
		case 1:
			x_next = 1;
			break;
		case 2:
			x_next = (x_prev * x_prev + 1) / (2 * x_prev);
			break;
		case 3:
			x_next = (2 * x_prev * x_prev * x_prev + 1) / (3 * x_prev * x_prev);
			break;
		case 4:
			x_next = (3 * x_prev * x_prev * x_prev * x_prev + 1) / (4 * x_prev * x_prev * x_prev);
			break;
		case 5:
			x_next = (4 * x_prev * x_prev * x_prev * x_prev * x_prev + 1) / 
		    		 (5 * x_prev * x_prev * x_prev * x_prev);
			break;
		case 6:
			x_next = (5 * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev + 1) / 
				 (6 * x_prev * x_prev * x_prev * x_prev * x_prev);
			break;
		case 7:
			x_next = (6 * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev + 1) / 
				 (7 * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev);
			break;
		case 8:
			x_next = (7 * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev + 1) / 
				 (8 * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev);
			break;
		case 9:
			x_next = (8 * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev + 1) / 
				 (9 * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev);
			break;
		default:
			fprintf(stderr, "unexpected degree\n");
			exit(1);
	}

	return x_next;
}

static inline
void newton_iteration(double complex x0, short *root_idx, short *n_its) {
	double complex x = x0;
	for (short i_iter = 0; i_iter < MAX_ITERATIONS; i_iter++) {
		x = newton_iteration_step(x, order);

		// TODO: "The absolute value of a complex number is the square root of its square norm. How can one avoid taking the square root? In particular, how can you avoid the use of the function cabs?"
		// TODO: "The square norm of a complex number is the sum of two squares. When computing it for a difference x - x', how can one avoid computing twice the difference of the respective real and imaginary parts?"

		// TODO: "It is possible to achieve complexity log(d) in the degree d. The biggest problem is that naive checking distances to roots is slow if the degree is large. One approach to solve this is to combine a Taylor expansion of x^d - 1 and a binary search"

		// check abort criteria
		if ((cabs(x) < CONVERGENCE_DIST) || 
			(abs(creal(x)) > MAX_VALUE) || 
			(abs(cimag(x)) > MAX_VALUE)) {
			// This won't converge anymore
			break;
		}

		// check convergence criteria
		for (short i_root = 0; i_root < order; i_root++) {
			// TODO: We should save which root is the closest 
			// then only check this root 
			// and if the distance is larger than half the distance between roots
			if (cabs(x - roots[i_root]) < CONVERGENCE_DIST) {
				// converged
				*root_idx = i_root;
				*n_its = i_iter+1;
				return;
			}
		}
	}

	// didn't converge
	*root_idx = -1;
	*n_its = MAX_ITERATIONS;
}

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

#include "global_vars.h"

static inline
TYPE_COMPLEX get_x0(short i_row, short i_col) {
    TYPE_COMPLEX x0 = 0;
    x0 +=    X_MIN_RE + (X_MAX_RE-X_MIN_RE) * i_col / image_size; // Real values are on the x-axis
    x0 += I*(X_MAX_IM - (X_MAX_IM-X_MIN_IM) * i_row / image_size); // Imaginary values are on the y-axis
    return x0;
}

static inline
TYPE_COMPLEX get_root_by_index(TYPE_ATTR i_root) {
    assert(0 <= i_root && i_root < order);
    return cexp(I*(2 * PI * i_root / order));
}

static inline
TYPE_COMPLEX newton_iteration_step(TYPE_COMPLEX x_prev, int degree) {

	/* "Inserting the Newton iteration step naively, you obtain x - (x^d - 1)/(d*x^(d-1)). How can you simplify it?""

	Analytical expression is 	

	           ((d - 1) * x^d + 1) 
	x_next = -----------------------
	               d * x^(d-1)

	Can be simplified to 
	               1 
	x_next = [ -------- + (d-1) * x ] / d
	            x^(d-1)
	*/

	TYPE_COMPLEX x_next;

	switch (degree) {
		case 1:
			x_next = 1;
			break;
		case 2:
			x_next = (1 / x_prev + x_prev) / 2;
			break;
		case 3:
			x_next = (1 / (x_prev * x_prev) + 2 * x_prev) / 3;
			break;
		case 4:
			x_next = (1 / (x_prev * x_prev * x_prev) + 3 * x_prev) / 4;
			break;
		case 5:
			x_next = (1 / (x_prev * x_prev * x_prev * x_prev) + 4 * x_prev) / 5;
			break;
		case 6:
			x_next = (1 / (x_prev * x_prev * x_prev * x_prev * x_prev) + 5 * x_prev) / 6;
			break;
		case 7:
			x_next = (1 / (x_prev * x_prev * x_prev * x_prev * x_prev * x_prev) + 6 * x_prev) / 7;
			break;
		case 8:
			x_next = (1 / (x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev) + 7 * x_prev) / 8;
			break;
		case 9:
			x_next = (1 / (x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev * x_prev) + 8 * x_prev) / 9;
			break;
		default:
			fprintf(stderr, "unexpected degree\n");
			exit(1);
	}

	return x_next;
}

// Hint: "The absolute value of a complex number is the square root of its square norm. How can one avoid taking the square root? In particular, how can you avoid the use of the function cabs?"
// Hint2: "The square norm of a complex number is the sum of two squares. When computing it for a difference x - x', how can one avoid computing twice the difference of the respective real and imaginary parts?"
static inline
TYPE_COORDS csquared(TYPE_COMPLEX x) {
	return creal(x)*creal(x) + cimag(x)*cimag(x);
}

static inline
void newton_iteration(TYPE_COMPLEX x0, TYPE_ATTR *root_idx, TYPE_CONV *n_its) {
	TYPE_COMPLEX x = x0;
	TYPE_ATTR closest_root = -1;
	TYPE_COORDS tmp_dist_squared = NAN;

	for (short i_iter = 0; i_iter < MAX_ITERATIONS; i_iter++) {
		x = newton_iteration_step(x, order);

		// Check abort criteria
		// cabs(x) = sqrt(sqare(x)) but sqrt is slow
		if ((csquared(x) < MAX_DIST_TO_ORIGIN_SQUARED) || 
			(fabs(creal(x)) > MAX_VALUE) || 
			(fabs(cimag(x)) > MAX_VALUE)) {
			// This won't converge anymore
			break;
		}

		// Check convergence criteria for the closest root
		if (closest_root != -1) {
			tmp_dist_squared = csquared(x - roots[closest_root]);
			// Check if it is close enough to the root
			if (tmp_dist_squared < CONVERGENCE_DIST_SQUARED) {
				// Converged
				*root_idx = closest_root;
				*n_its = i_iter+1;
				return;
			}
			// Check if it is still the closest root
			if (tmp_dist_squared < half_root_distance_squared) {
				continue;
			} else {
				closest_root = -1;
			}
		}

		// If the closest root is not up to date check all of them
		for (TYPE_ATTR i_root = 0; i_root < order; i_root++) {
			tmp_dist_squared = csquared(x - roots[i_root]);
			// Check for convergence of each root
			if (tmp_dist_squared < CONVERGENCE_DIST_SQUARED) {
				// Converged
				*root_idx = i_root;
				*n_its = i_iter+1;
				return;
			} 
			// If it is close enough declare it as the closest root
			if (tmp_dist_squared < half_root_distance_squared) {
				closest_root = i_root;
				break;
			}
		}
	}

	// Didn't converge
	*root_idx = -1;
	*n_its = 0;
}

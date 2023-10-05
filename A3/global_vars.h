#pragma once

#include <complex.h>

/////////////////////////////////////////////
// Constants
// TODO: Implement the better types
#define TYPE_ATTR char
#define TYPE_CONV unsigned char

#define PI 3.14159265358979323846

#define X_MIN_RE -2.
#define X_MIN_IM -2.
#define X_MAX_RE +2.
#define X_MAX_IM +2.

#define DEFAULT_THREADS 4
#define DEFAULT_IMAGE_SIZE 1000
#define DEFAULT_ORDER 3

#define MAX_THREADS 10
#define MAX_IMAGE_SIZE 100000
#define MAX_ORDER 9

#define MAX_ITERATIONS 128
#define MAX_DIST_TO_ORIGIN 1e-3
#define MAX_DIST_TO_ORIGIN_SQUARED 1e-6
#define MAX_VALUE 1e10
#define CONVERGENCE_DIST 1e-3
#define CONVERGENCE_DIST_SQUARED 1e-6



/////////////////////////////////////////////
// Global variables
int n_threads = DEFAULT_THREADS;
int image_size = DEFAULT_IMAGE_SIZE;
int order = DEFAULT_ORDER;

short **root_idxs;
short **n_its;
double complex *roots;

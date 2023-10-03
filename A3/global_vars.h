#pragma once

/////////////////////////////////////////////
// Constants
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
#define MAX_ORDER 10


/////////////////////////////////////////////
// Global variables
int n_threads = DEFAULT_THREADS;
int image_size = DEFAULT_IMAGE_SIZE;
int order = DEFAULT_ORDER;
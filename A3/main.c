#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

#include "constants.h"


// global variables
short d;
short nlines;
short nthreads;

double complex get_x0(short i_row, short i_col) {
    double complex x0 = 0;
    x0 +=    X_MIN_RE + (X_MAX_RE-X_MIN_RE) * i_row / nlines;
    x0 += I*(X_MIN_IM + (X_MAX_IM-X_MIN_IM) * i_row / nlines);
    return x0;
}

double complex get_root_by_index(short i_root) {
    assert(0 < i_root && i_root < d);
    return exp(I*(2 * PI * i_root / d));
}

int main(int argc, char *argv[]){

    return 0;
}
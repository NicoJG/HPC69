#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

#include "global_vars.h"

double complex get_x0(short i_row, short i_col) {
    double complex x0 = 0;
    x0 +=    X_MIN_RE + (X_MAX_RE-X_MIN_RE) * i_row / image_size;
    x0 += I*(X_MIN_IM + (X_MAX_IM-X_MIN_IM) * i_col / image_size);
    return x0;
}

double complex get_root_by_index(short i_root) {
    assert(0 < i_root && i_root < order);
    return exp(I*(2 * PI * i_root / order));
}
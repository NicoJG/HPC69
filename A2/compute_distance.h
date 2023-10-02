#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "constants.h"

static inline
short 
euc_distance(
    Coordinate p1, 
    Coordinate p2
    ) {
        int dx, dy, dz;

        dx = p2.x - p1.x;
        dy = p2.y - p1.y;
        dz = p2.z - p1.z;

        float d = sqrtf((float)(
            dx * dx + 
            dy * dy + 
            dz * dz
            ));

        // truncation
        d /= 10;

        return (short) d;

    };

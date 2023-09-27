#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cell_distances.h"

// Right now we still use compute_distances() in test.c, but we don't need it in the main file. Should remove later.
void 
compute_distances(
    Coordinate *coords,
    int num_cords,
    short *distances
) {
    size_t count = 0;
    for (size_t ix = 0; ix < num_cords - 1; ++ix) {
        for (size_t jx = ix + 1; jx < num_cords; ++jx) {
            distances[count++] = euc_distance(coords[ix], coords[jx]);
        }
    }
};

short 
euc_distance(
    Coordinate p1, 
    Coordinate p2
    ) {
        float x1, x2, y1, y2, z1, z2, d;

        x1 = (float) p1.x;
        x2 = (float) p2.x;
        y1 = (float) p1.y;
        y2 = (float) p2.y;
        z1 = (float) p1.z;
        z2 = (float) p2.z;

        d = sqrtf(
            (x2 - x1) * (x2 - x1) + 
            (y2 - y1) * (y2 - y1) + 
            (z2 - z1) * (z2 - z1)
            );
        
        d = roundf(d/10); // Why do we divide by 10? /N
        return (short) d;

    };

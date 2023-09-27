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
        int dx, dy, dz;

        dx = p2.x - p1.x;
        dy = p2.y - p1.y;
        dz = p2.z - p1.z;

        float d = sqrtf((float)(
            dx * dx + 
            dy * dy + 
            dz * dz
            ));

        // round to 2 decimal places (faster than roundf)
        d = (d+5)/10;

        return (short) d;

    };

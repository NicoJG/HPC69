#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cell_distances.h"

void 
compute_distances(
    Coordinate *coords,
    int num_cords,
    int *distances
) {
    size_t count = 0;
    for (size_t ix = 0; ix < num_cords - 1; ++ix) {
        for (size_t jx = ix + 1; jx < num_cords; ++jx) {
            distances[count++] = euc_distance(coords[ix], coords[jx]);
        }
    }
};

int 
euc_distance(
    Coordinate p1, 
    Coordinate p2
    ) {
        float x1, x2, y1, y2, z1, z2, d;
        x1 = (float) p1.x / 10000;
        x2 = (float) p2.x / 10000;
        y1 = (float) p1.y / 10000;
        y2 = (float) p2.y / 10000;
        z1 = (float) p1.z / 10000;
        z2 = (float) p2.z / 10000;

        d = sqrtf(
            (x2 - x1) * (x2 - x1) + 
            (y2 - y1) * (y2 - y1) + 
            (z2 - z1) * (z2 - z1)
            );
        
        d = (100 * roundf(d)) / 100;

        return (int) d * 10000;

    };
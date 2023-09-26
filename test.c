#include <stdlib.h>
#include <stdio.h>

#include "cell_distances.h"

int 
main(
    int argc,
    char *argv[]
) {

    Coordinate points[] = {
        {-5240, -3380, -2570},
        {9060, 4180, 8450},
        {5470, -3420, -7470}
    };

    int n = 3;
    short dist[3] = {0, 0, 0};

    compute_distances(points, n, dist);
    printf("dist[0]: %d\n", dist[2]);

    return 0;

}
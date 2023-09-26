#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FLOAT_LENGTH 8
#define COORD_NBR 1
#define NBR_LINES 10

typedef struct {
	short x; // -32,767 to +32,767
	short y;
	short z;
} Coordinate;

Coordinate 
parse_coordinate(
    const char *buffer, 
    int which_coord
    );

int 
parse_value(
    const char *buffer
    );

void 
read_coordinates(
    FILE *fp, 
    Coordinate *coords, 
    size_t buffer_size
    );

void 
compute_distances(
    Coordinate *coords,
    int num_cords,
    int *distances
    );

int 
euc_distance(
    Coordinate p1, 
    Coordinate p2
    );
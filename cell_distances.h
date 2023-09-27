#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FLOAT_LENGTH 8
#define BYTES_PER_LINE 3 * FLOAT_LENGTH
#define COORD_NBR 1
#define NBR_LINES 100
#define MAX_DISTANCE 3464

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
    char *buffer,
    size_t buffer_size
    );

void 
compute_distances(
    Coordinate *coords,
    int num_cords,
    short *distances
    );

short 
euc_distance(
    Coordinate p1, 
    Coordinate p2
    );

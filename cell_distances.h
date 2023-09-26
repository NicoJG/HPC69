#include <stdlib.h>
#include <stdio.h>

#define FLOAT_LENGTH 8
#define BUFFER_SZ 512
#define COORD_NBR 1

typedef struct {
	int x;
	int y;
	int z;
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


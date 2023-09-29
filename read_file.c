#include <stdio.h>
#include <stdlib.h>

#include "cell_distances.h"

void 
read_coordinates (
    FILE *fp, Coordinate *coords, char *buffer, int buffer_size)
{
    fread(buffer, buffer_size, 1, fp);

    int coord_x = 0;
    for (int ix = 0; ix < buffer_size; ix += BYTES_PER_LINE){
	    coords[coord_x++] = parse_coordinate(buffer, ix);
    }
}

Coordinate parse_coordinate(const char *buffer, int which_coord) {
    Coordinate coord;

    coord.x = parse_value(buffer + which_coord);
    coord.y = parse_value(buffer + which_coord + FLOAT_LENGTH);
    coord.z = parse_value(buffer + which_coord + 2 * FLOAT_LENGTH);
    
    return coord;
}

short parse_value(const char *buffer) {
    short sign = (buffer[0] == '-') ? -1 : 1;

    return sign * (
        (buffer[1] - '0') * 10000 + 
        (buffer[2] - '0') * 1000 +
        (buffer[4] - '0') * 100 + 
        (buffer[5] - '0') * 10 +
        (buffer[6] - '0')
    );
}


#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "constants.h"

static inline
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

static inline
Coordinate parse_coordinate(const char *buffer, int which_coord) {
    Coordinate coord;

    coord.x = parse_value(buffer + which_coord);
    coord.y = parse_value(buffer + which_coord + FLOAT_LENGTH);
    coord.z = parse_value(buffer + which_coord + 2 * FLOAT_LENGTH);
    
    return coord;
}

static inline
void 
read_coordinates (
    FILE *fp, int byte_position, Coordinate *coords, char *buffer, int buffer_size)
{
	fseek(fp, byte_position, SEEK_SET);
    fread(buffer, buffer_size, 1, fp);

    int coord_x = 0;
    for (int ix = 0; ix < buffer_size; ix += BYTES_PER_LINE){
	    coords[coord_x++] = parse_coordinate(buffer, ix);
    }
}



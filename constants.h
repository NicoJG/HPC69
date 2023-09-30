#pragma once

#define FLOAT_LENGTH 8
#define BYTES_PER_LINE (3 * FLOAT_LENGTH)
#define COORD_NBR 1
#define MAX_DISTANCE 3464
#define MAX_MEM 4500000 //Ideally 4.5Mb, change that when it works. 
// MAX_MEM has to be smaller than 4294967296 (UINT_MAX)

#define INPUT_FILE "data/cells_1e4"

typedef struct {
	short x; // -32,767 to +32,767
	short y;
	short z;
} Coordinate;

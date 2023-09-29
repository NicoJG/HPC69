#pragma once

#define FLOAT_LENGTH 8
#define BYTES_PER_LINE 3 * FLOAT_LENGTH
#define COORD_NBR 1
#define NBR_LINES 1000
#define MAX_DISTANCE 3464

#define INPUT_FILE "data/cells_1e5"

typedef struct {
	short x; // -32,767 to +32,767
	short y;
	short z;
} Coordinate;

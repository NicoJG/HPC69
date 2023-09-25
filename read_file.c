#include <stdio.h>
#include <stdlib.h>

#define FLOAT_LENGTH 8
#define BUFFER_SZ 512
#define COORD_NBR 10 // Could change to BUFFER_SZ / ( 3 * 8 )

typedef struct {
	int x;
	int y;
	int z;
} Coordinate;

Coordinate parse_coordinate(const char *buffer, int which_coord);
int parse_value(const char *buffer);

int 
main (
    int argc,
    char *argv[]
) {

    FILE *fp;
    char *file_name = "data/cells_1e2";

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        perror("File could not be opened.");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    int nbr_buffer = file_size/BUFFER_SZ;
    printf("Number of buffers: %d\n", nbr_buffer);
    fseek(fp, 0, SEEK_SET);

    char buffer[BUFFER_SZ];
    fread(buffer, sizeof(buffer), 1, fp);

    Coordinate *coords = (Coordinate*) malloc(COORD_NBR * sizeof(Coordinate));
    if (!coords){
	perror("Failed to allocte memory for coodinates.");
	fclose(fp);
	return 1;
    }

    size_t num_coords = 0;

    for (size_t ix = 0; ix < COORD_NBR * 3 * FLOAT_LENGTH; ix += 3 * FLOAT_LENGTH){
	    coords[num_coords++] = parse_coordinate(buffer, ix);
    }

    //COMPUTE DISTANCES
    
    for (size_t ix = 0; ix < num_coords; ix++) {
	printf("%d, %d, %d\n", coords[ix].x, coords[ix].y, coords[ix].z);
    }
   
    printf("The file has size %d\n", file_size);
 
    free(coords);
    fclose(fp);
    
    return 0;

}

Coordinate parse_coordinate(const char *buffer, int which_coord) {
    Coordinate coord;

    coord.x = parse_value(buffer + which_coord);
    coord.y = parse_value(buffer + which_coord + FLOAT_LENGTH);
    coord.z = parse_value(buffer + which_coord + 2 * FLOAT_LENGTH);

    return coord;
}

int parse_value(const char *buffer) {
    int sign = (buffer[0] == '-') ? -1 : 1;

    return sign * (
        (buffer[1] - '0') * 10000 + 
        (buffer[2] - '0') * 1000 +
        (buffer[4] - '0') * 100 + 
        (buffer[5] - '0') * 10 +
        (buffer[6] - '0')
    );
}


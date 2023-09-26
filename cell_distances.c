#include <stdio.h>
#include <stdlib.h>
#include "cell_distances.h"

int main(int argc, char *argv[]){

	FILE *fp;
	char *file_name = "data/cells_1e2";

	fp = fopen(file_name, "rb");
	if (fp == NULL) {
	perror("File could not be opened.");
	return 1;
	}

	int buffer_size = 24 * NBR_LINES;

	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	int nbr_buffer = file_size/buffer_size;
	printf("File size: %d\n", file_size);
	printf("Buffer size: %d\n", buffer_size);
	printf("Number of buffers: %d\n", nbr_buffer);
	fseek(fp, 0, SEEK_SET);

	Coordinate *coords = (Coordinate*) malloc(sizeof(Coordinate) * NBR_LINES);
	if (!coords){
		perror("Failed to allocte memory for coordinates.");
		fclose(fp);
		return 1;
	}

	// Read the coordinates
	for (size_t count_buffer = 0; count_buffer < nbr_buffer; count_buffer++){
		read_coordinates(fp, coords, buffer_size);	
		for (size_t ix = 0; ix < COORD_NBR; ix++) {
			printf("%d, %d, %d\n", coords[ix].x, coords[ix].y, coords[ix].z);
		}
	}

	int remaining_bytes = file_size % buffer_size;
	read_coordinates(fp, coords, remaining_bytes); // Maybe we should reallocate for the exact size of remaining bytes.

	free(coords);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include "cell_distances.h"

int main(int argc, char *argv[]){

	FILE *fp;
	char *file_name = "data/cells_3e7";

	fp = fopen(file_name, "rb");
	if (fp == NULL) {
		perror("File could not be opened.");
		return 1;
	}

	int buffer_size = BYTES_PER_LINE * NBR_LINES;

	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	int nbr_buffer = file_size/buffer_size; 
	printf("File size: %d bytes\n", file_size);
	printf("Buffer size: %d bytes\n", buffer_size);
	printf("Number of buffers: %d\n", nbr_buffer);
	fseek(fp, 0, SEEK_SET);

	Coordinate *coords = (Coordinate*) malloc(sizeof(Coordinate) * NBR_LINES); // So far we've been working with short variable type for the coordinates. But since we have to convert it to float to compute the distances, maybe we should save it as float anyway. Uses more memory but I guess it should be faster I believe. /R
	if (!coords){
		perror("Failed to allocte memory for coordinates.");
		fclose(fp);
		return 1;
	}

	short dist;
	short distances[MAX_DISTANCE+1]; //Array to store all the possible distances (from 0 to 3464).
	// TODO: we should probably use heap memory here /N
	memset(distances, 0, sizeof(distances)); // Set all the distances count to 0.

	// TODO: We need to compute all distances, not only in one buffer

	// Read the coordinates
	for (size_t count_buffer = 0; count_buffer < nbr_buffer; count_buffer++){
		read_coordinates(fp, coords, buffer_size);
		
		for (size_t ix = 0; ix < NBR_LINES - 1; ++ix) {
			for (size_t jx = ix + 1; jx < NBR_LINES; ++jx) {
			    dist = euc_distance(coords[ix], coords[jx]);
			    //printf("dist = %d\n", dist);
			    distances[dist]++;
			}
		}
	}

	int remaining_bytes = file_size % buffer_size;
	read_coordinates(fp, coords, remaining_bytes); // Maybe we should reallocate for the exact size of remaining bytes.
	for (size_t ix = 0; ix < NBR_LINES - 1; ++ix) {
		for (size_t jx = ix + 1; jx < NBR_LINES; ++jx) {
		    dist = euc_distance(coords[ix], coords[jx]);
		    distances[dist]++;
		}
	}

	for (size_t ix = 0; ix < 3464; ix++) {
		if (distances[ix] > 2)
			printf("%d %d\n", ix, distances[ix]);
	}
	
	fclose(fp);
	free(coords);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include "cell_distances.h"

int main(int argc, char *argv[]){

	FILE *fp;
	char *file_name = "data/cells_1e4";

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

	char *buffer = (char*)malloc(buffer_size);
	Coordinate *coords = (Coordinate*) malloc(sizeof(Coordinate) * NBR_LINES); 	
	// So far we've been working with short variable type for the coordinates. But since we have to convert it to float to compute the distances, maybe we should save it as float anyway. Uses more memory but I guess it should be faster I believe. /R
	// But actually they say in the description we shouldn't use float so idk. /R
	if (!coords){
		perror("Failed to allocte memory for coordinates.");
		fclose(fp);
		return 1;
	}

	short dist;
	short distances[MAX_DISTANCE+1]; //Array to store all the possible distances (from 0 to 3464).
	memset(distances, 0, sizeof(distances)); // Set all the distances count to 0.

	// TODO: We need to compute all distances, not only within each buffer

	// Read the coordinates
	for (size_t count_buffer = 0; count_buffer < 1; count_buffer++){
	// for (size_t count_buffer = 0; count_buffer < nbr_buffer; count_buffer++){
		read_coordinates(fp, coords, buffer, buffer_size);
		for (size_t ix = 0; ix < NBR_LINES - 1; ++ix) {
			for (size_t jx = ix + 1; jx < NBR_LINES; ++jx) {
			    dist = euc_distance(coords[ix], coords[jx]);
			    //printf("dist = %d\n", dist);
			    distances[dist]++;
			}
		}
	}

	int remaining_bytes = file_size % buffer_size;
	read_coordinates(fp, coords, buffer, remaining_bytes); // Maybe we should reallocate for the exact size of remaining bytes.
	for (size_t ix = 0; ix < NBR_LINES - 1; ++ix) {
		for (size_t jx = ix + 1; jx < NBR_LINES; ++jx) {
		    dist = euc_distance(coords[ix], coords[jx]);
		    distances[dist]++;
		}
	}
	int count_distances = 0;
	int total_lines = file_size/24;

	for (size_t ix = 0; ix <= MAX_DISTANCE; ix++) {
		count_distances += distances[ix];
		if (distances[ix] > 0)
			printf("%d %d\n", ix, distances[ix]);
	}
	
	printf("Counted distances: %d\n", count_distances);
	printf("Total distances: %d\n", total_lines * (total_lines + 1) / 2); // Sum of n integers

	fclose(fp);
	free(buffer);
	free(coords);
	return 0;
}

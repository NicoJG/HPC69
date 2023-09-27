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

	char *buffer_fixed = (char*)malloc(buffer_size);
	char *buffer_scan = (char*)malloc(buffer_size);
	Coordinate *coords_fixed = (Coordinate*) malloc(sizeof(Coordinate) * NBR_LINES); 	
	Coordinate *coords_scan = (Coordinate*) malloc(sizeof(Coordinate) * NBR_LINES); 	
	// So far we've been working with short variable type for the coordinates. But since we have to convert it to float to compute the distances, maybe we should save it as float anyway. Uses more memory but I guess it should be faster I believe. /R
	// But actually they say in the description we shouldn't use float so idk. /R
	if (!buffer_fixed || !buffer_scan || !coords_fixed || !coords_scan){
		perror("Failed to allocte memory for coordinates or buffer.");
		fclose(fp);
		return 1;
	}

	short dist;
	int count_distances[MAX_DISTANCE+1]; //Array to store all the possible distances (from 0 to 3464).
	memset(count_distances, 0, sizeof(count_distances)); // Set all the distances count to 0.

	int remaining_bytes = file_size % buffer_size;
	int remaining_lines = remaining_bytes / BYTES_PER_LINE;

	// TODO: We need to compute all distances, not only within each buffer

	// Read the coordinates

	// Move the "fixed" buffer -> If needed could try reversed sweep but complicated
	for (size_t i_fixed = 0; i_fixed < nbr_buffer; i_fixed++){
		fseek(fp, i_fixed * buffer_size, SEEK_SET);
		read_coordinates(fp, coords_fixed, buffer_fixed, buffer_size);

		// Distances within fixed buffer
		for (size_t ix = 0; ix < NBR_LINES - 1; ++ix) {
			for (size_t jx = ix + 1; jx < NBR_LINES; ++jx) {
			    dist = euc_distance(coords_fixed[ix], coords_fixed[jx]);
			    count_distances[dist]++;
			}
		}

		// Scan over the rest of the file
		// if i_fixed is the last one this for loop is automatically skipped
		for (size_t i_scan = i_fixed + 1; i_scan < nbr_buffer; i_scan++) {
			fseek(fp, i_scan * buffer_size, SEEK_SET);
			read_coordinates(fp, coords_scan, buffer_scan, buffer_size);

			// Distances from fixed to scan buffer
			for (size_t ix = 0; ix < NBR_LINES; ++ix) {
				for (size_t jx = 0; jx < NBR_LINES; ++jx) {
					dist = euc_distance(coords_fixed[ix], coords_scan[jx]);
					count_distances[dist]++;
				}
			}

		}

		// Scan over the last buffer of the file
		fseek(fp, nbr_buffer * buffer_size, SEEK_SET);
		read_coordinates(fp, coords_scan, buffer_scan, remaining_bytes);
		for (size_t ix = 0; ix < NBR_LINES; ++ix) {
			for (size_t jx = 0; jx < remaining_lines; ++jx) {
				dist = euc_distance(coords_fixed[ix], coords_scan[jx]);
				count_distances[dist]++;
			}
		}

		printf("buffer %i/%i\n", i_fixed+1, nbr_buffer);
	}

	// Distances within remaining buffer
	for (short ix = 0; ix < remaining_lines - 1; ++ix) {
		for (short jx = ix + 1; jx < remaining_lines; ++jx) {
			dist = euc_distance(coords_scan[ix], coords_scan[jx]);
			count_distances[dist]++;
		}
	}
	
	long int total_count_distances = 0;
	long int total_lines = file_size/24;

	for (size_t ix = 0; ix < MAX_DISTANCE + 1; ix++) {
		total_count_distances += count_distances[ix];
		printf("%d %d\n", ix, count_distances[ix]);
	}
	
	printf("Counted distances: %d\n", total_count_distances);
	printf("Total distances: %d\n", (total_lines-1) * (total_lines) / 2); // Sum of n-1 integers

	fclose(fp);
	free(buffer_fixed);
	free(buffer_scan);
	free(coords_fixed);
	free(coords_scan);
	return 0;
}

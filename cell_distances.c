#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <omp.h>

#include "read_file.h"
#include "compute_distance.h"
#include "constants.h"

int parse_num_threads(int argc, char *argv[]) {
	int n_threads = 4;
	int opt, val;
	if (argc > 1) {
		while((opt = getopt(argc, argv, "t:")) != -1) {
			switch(opt) {
				case 't':
					if ((val = atoi(optarg)) == 0) {
						printf("'%s' is not a valid integer, using default number of threads, %d.\n", optarg, n_threads);
						break;
					}
					n_threads = val;
					printf("Using %d threads.\n", n_threads);
					break;
				case '?':
					printf("Unknown option, using default number of threads, %d.\n", n_threads);
					break;
			}
		}
	} else {
		printf("Using default number of threads, %d.\n", n_threads);
	}
}

void determine_file_and_buffer_size(
				FILE* fp, 
				int* file_size, 
				int* buffer_size, 
				int* nbr_buffer, 
				int* nbr_lines
				)
{
	// Get file size
	fseek(fp, 0, SEEK_END);
	*file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (*file_size % BYTES_PER_LINE != 0){
		printf("File size is not divisible by the number of bytes per lines: %d\n Either the format is not xx.xxx or a coordinate is missing.", BYTES_PER_LINE);
		exit(1);
	}

	// Determine buffer size
	if (*file_size > MAX_MEM){
		*buffer_size = 2 * MAX_MEM / 3 - ((2 * MAX_MEM / 3) % BYTES_PER_LINE);
	}
	else{
		*buffer_size = *file_size; 
	}

	*nbr_buffer = *file_size / *buffer_size; 
	*nbr_lines = *buffer_size / BYTES_PER_LINE;

	printf("File size: %d bytes\n", *file_size);
	printf("Buffer size: %d bytes\n", *buffer_size);
	printf("Number of buffers: %d\n", *nbr_buffer);
	printf("Number of lines per buffer: %d lines\n", *nbr_lines);
}

void count_distances_within(Coordinate* coords, int nbr_coords, int* count_distances) {
	// #pragma omp parallel for collapse(2) reduction(+:count_distances[:3465])
	for (long int ix = 0; ix < nbr_coords - 1; ++ix) {
		for (long int jx = ix + 1; jx < nbr_coords; ++jx) {
			short dist = euc_distance(coords[ix], coords[jx]);
			count_distances[dist] += 1;
		}
	}
}

void count_distances_between(Coordinate* coords1, Coordinate* coords2, int nbr_coords1, int nbr_coords2, int* count_distances) {
	// #pragma omp parallel for collapse(2) reduction(+:count_distances[:3465])
	for (long int ix = 0; ix < nbr_coords1; ++ix) {
		for (long int jx = 0; jx < nbr_coords2; ++jx) {
			short dist = euc_distance(coords1[ix], coords2[jx]);
			count_distances[dist] += 1;
		}
	}
}


int main(int argc, char *argv[]){

	// Set number of threads
	int n_threads = parse_num_threads(argc, argv);
	omp_set_num_threads(n_threads);

	// Open file
	FILE *fp;
	fp = fopen(INPUT_FILE, "rb");
	if (fp == NULL) {
		perror("File could not be opened.");
		return 1;
	}

	// Get file and buffer size
	int file_size, buffer_size, nbr_buffer, nbr_lines;
	determine_file_and_buffer_size(fp, &file_size, &buffer_size, &nbr_buffer, &nbr_lines);
	int remaining_bytes = file_size % buffer_size;
	int remaining_lines = remaining_bytes / BYTES_PER_LINE;

	// Allocate arrays
	char *buffer = (char*)malloc(buffer_size);
	Coordinate *coords_fixed = (Coordinate*) malloc(sizeof(Coordinate) * nbr_lines); 	
	Coordinate *coords_scan = (Coordinate*) malloc(sizeof(Coordinate) * nbr_lines); 	
	if (!buffer || !coords_fixed || !coords_scan){
		perror("Failed to allocte memory for coordinates or buffer.");
		fclose(fp);
		return 1;
	}
	// Should we change the counts to long int? (INT_MAX = 2e9) /N
	int count_distances[MAX_DISTANCE+1] = {0}; //Array to store all the possible distances (from 0 to 3464).

	///////////////////////////////////////////
	// Main computations

	// Move the "fixed" buffer -> If needed could try reversed sweep but complicated
	for (int i_fixed = 0; i_fixed < nbr_buffer; i_fixed++){
		// Read fixed buffer
		read_coordinates(fp, (i_fixed * buffer_size), coords_fixed, buffer, buffer_size);

		// Distances within fixed buffer
		count_distances_within(coords_fixed, nbr_lines, count_distances);

		// Scan over the rest of the file
		// (if i_fixed is the last one this for loop is automatically skipped)
		for (int i_scan = i_fixed + 1; i_scan < nbr_buffer; i_scan++) {
			read_coordinates(fp, (i_scan * buffer_size), coords_scan, buffer, buffer_size);
			count_distances_between(coords_fixed, coords_scan, nbr_lines, nbr_lines, count_distances);
		}

		// Scan over the last buffer of the file
		read_coordinates(fp, (nbr_buffer * buffer_size), coords_scan, buffer, remaining_bytes);
		count_distances_between(coords_fixed, coords_scan, nbr_lines, remaining_lines, count_distances);

		// Show progress
		printf("buffer %i/%i\n", i_fixed+1, nbr_buffer);
	}
	// Distances within remaining buffer
	count_distances_within(coords_scan, remaining_lines, count_distances);

	///////////////////////////////////////////
	
	// Check if we counted all pairs
	long int total_count_distances = 0;
	long int total_lines = file_size/24;
	for (int ix = 0; ix < MAX_DISTANCE + 1; ix++) {
		total_count_distances += count_distances[ix];
		printf("%05.2f %d\n", ((float)ix)/100, count_distances[ix]);
	}
	printf("Counted distances: %ld\n", total_count_distances);
	printf("Total distances: %ld\n", (total_lines-1) * (total_lines) / 2); // Sum of n-1 integers

	// Free stuff
	fclose(fp);
	free(buffer);
	free(coords_fixed);
	free(coords_scan);
	
	return 0;
}

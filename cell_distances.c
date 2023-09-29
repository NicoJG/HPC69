#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <omp.h>

#include "read_file.h"
#include "compute_distance.h"
#include "constants.h"

int main(int argc, char *argv[]){

	// Set number of threads
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

	omp_set_num_threads(n_threads);

	// Read file
	FILE *fp;
	char *file_name = INPUT_FILE;

	fp = fopen(file_name, "rb");
	if (fp == NULL) {
		perror("File could not be opened.");
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);

	if (file_size % BYTES_PER_LINE != 0){
		printf("File size is not divisible by the number of bytes per lines: %d\n Either the format is not xx.xxx or a coordinate is missing.", BYTES_PER_LINE);
		return 1;
	}

	int buffer_size;
	if (file_size > MAX_MEM){
		buffer_size = 2 * MAX_MEM / 3 - ((2 * MAX_MEM / 3) % BYTES_PER_LINE);
	}
	else{
		buffer_size = file_size; 
	}

	int nbr_buffer = file_size/buffer_size; 
	int nbr_lines = buffer_size / BYTES_PER_LINE;

	printf("File size: %d bytes\n", file_size);
	printf("Buffer size: %d bytes\n", buffer_size);
	printf("Number of buffers: %d\n", nbr_buffer);
	printf("Number of lines per buffer: %d lines\n", nbr_lines);
	fseek(fp, 0, SEEK_SET);

	char *buffer = (char*)malloc(buffer_size);
	
	Coordinate *coords_fixed = (Coordinate*) malloc(sizeof(Coordinate) * nbr_lines); 	
	Coordinate *coords_scan = (Coordinate*) malloc(sizeof(Coordinate) * nbr_lines); 	

	if (!buffer || !coords_fixed || !coords_scan){
		perror("Failed to allocte memory for coordinates or buffer.");
		fclose(fp);
		return 1;
	}

	short dist;
	int count_distances[MAX_DISTANCE+1]; //Array to store all the possible distances (from 0 to 3464).
	memset(count_distances, 0, sizeof(int)*(MAX_DISTANCE+1)); // Set all the distances count to 0.

	int remaining_bytes = file_size % buffer_size;
	int remaining_lines = remaining_bytes / BYTES_PER_LINE;

	// Read the coordinates

	// Move the "fixed" buffer -> If needed could try reversed sweep but complicated
	for (int i_fixed = 0; i_fixed < nbr_buffer; i_fixed++){
		fseek(fp, i_fixed * buffer_size, SEEK_SET);
		read_coordinates(fp, coords_fixed, buffer, buffer_size);

		// Distances within fixed buffer
		// #pragma omp parallel for collapse(2) reduction(+:count_distances[:3465])
		for (long int ix = 0; ix < nbr_lines - 1; ++ix) {
			for (long int jx = ix + 1; jx < nbr_lines; ++jx) {
			    dist = euc_distance(coords_fixed[ix], coords_fixed[jx]);
				// #pragma omp atomic
			    count_distances[dist] += 1;
			}
		}

		// Scan over the rest of the file
		// if i_fixed is the last one this for loop is automatically skipped
		for (int i_scan = i_fixed + 1; i_scan < nbr_buffer; i_scan++) {
			fseek(fp, i_scan * buffer_size, SEEK_SET);
			read_coordinates(fp, coords_scan, buffer, buffer_size);

			// Distances from fixed to scan buffer
		// #pragma omp parallel for collapse(2) reduction(+:count_distances[:3465])
			for (long int ix = 0; ix < nbr_lines; ++ix) {
				for (long int jx = 0; jx < nbr_lines; ++jx) {
					dist = euc_distance(coords_fixed[ix], coords_scan[jx]);
					// #pragma omp atomic
					count_distances[dist] += 1;
				}
			}

		}

		// Scan over the last buffer of the file
		fseek(fp, nbr_buffer * buffer_size, SEEK_SET);
		read_coordinates(fp, coords_scan, buffer, remaining_bytes);
		// #pragma omp parallel for collapse(2) reduction(+:count_distances[:3465])
		for (long int ix = 0; ix < nbr_lines; ++ix) {
			for (long int jx = 0; jx < remaining_lines; ++jx) {
				dist = euc_distance(coords_fixed[ix], coords_scan[jx]);
				// #pragma omp atomic
				count_distances[dist] += 1;
			}
		}

		printf("buffer %i/%i\n", i_fixed+1, nbr_buffer);
	}

	// Distances within remaining buffer
	// #pragma omp parallel for collapse(2) reduction(+:count_distances[:3465])
	for (long int ix = 0; ix < remaining_lines - 1; ++ix) {
		for (long int jx = ix + 1; jx < remaining_lines; ++jx) {
			dist = euc_distance(coords_scan[ix], coords_scan[jx]);
			count_distances[dist] += 1;
		}
	}
	
	long int total_count_distances = 0;
	long int total_lines = file_size/24;

	for (int ix = 0; ix < MAX_DISTANCE + 1; ix++) {
		total_count_distances += count_distances[ix];
		printf("%05.2f %d\n", ((float)ix)/100, count_distances[ix]);
	}
	
	printf("Counted distances: %ld\n", total_count_distances);
	printf("Total distances: %ld\n", (total_lines-1) * (total_lines) / 2); // Sum of n-1 integers

	fclose(fp);
	free(buffer);
	free(coords_fixed);
	free(coords_scan);
	
	return 0;
}

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "global_vars.h"

void parse_cmd_args(int argc, char *argv[]) {
    int opt, val;

	if (argc > 1) {
		while((opt = getopt(argc, argv, "t:l:")) != -1) {
			switch(opt) {
				case 't':
					val = atoi(optarg);
					if (val == 0) {
						printf("'%s' is not a valid integer... Using default number of threads, %d.\n", optarg, DEFAULT_THREADS);
						n_threads = DEFAULT_THREADS;
						break;
					}
					else if (val > MAX_THREADS){
						printf("Too many threads (%d)... Using default maximum value: %d\n", val, MAX_THREADS);
						n_threads = MAX_THREADS;
						break;
					}
					n_threads = val;
					
					break;
				case 'l':
					val = atoi(optarg);
					if (val == 0) {
						printf("Invalid argument for option -l. '%s' is not a valid integer ... Using default image size, %d.\n", optarg, DEFAULT_IMAGE_SIZE);
						image_size = DEFAULT_IMAGE_SIZE;
						break;
					}
					else if (val > MAX_IMAGE_SIZE){
						printf("Image size is too big (%d)... Using default maximum value: %d\n", val, MAX_IMAGE_SIZE);
						image_size = MAX_IMAGE_SIZE;
						break;
					}
					image_size = val;
					break;

				case '?':
					if (optopt == 't'){
						printf ("Option -%c requires an argument\n", optopt);
					 	exit(-1);
					} 
                    else if (optopt >= ' ' && optopt <= '~'){
						printf ("Unknown option `-%c'\n", optopt);
						exit(-1);
					}
				    else{
						printf ("Unknown option character `\\x%x'\n",optopt);
						exit(-1);
					}
				}
					
			}
		if (optind < argc) {
			order = atoi(argv[optind]);
			if (order > 9){
				printf("The order given (%d) is too large... Using default maximum value: %d\n", order, MAX_ORDER);
				order = MAX_ORDER;
			}
		}
    }

	// The program will fail if n_threads > image_size
	if (n_threads > image_size) {
		fprintf(stderr, "you cant assign more threads than rows and cols in the image.\n");
		exit(1);
	}

	printf("Using %d threads\n", n_threads);
	printf("Image size is %d\n", image_size);
	printf("Order is %d\n", order);

}

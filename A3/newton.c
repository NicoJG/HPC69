#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <threads.h>
#include <ctype.h>

#define MAX_THREADS 10
#define MAX_IMAGE_SIZE 100000
#define MAX_ORDER 10

#define DEFAULT_THREADS 4
#define DEFAULT_IMAGE_SIZE 1000
#define DEFAULT_ORDER 3

int main(int argc, char *argv[]){
	
	//Default values
	int n_threads = DEFAULT_THREADS;
	int image_size = DEFAULT_IMAGE_SIZE;
	int order = DEFAULT_ORDER;
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
					 	return -1;
					 }
				    	else if (isprint (optopt)){
						printf ("Unknown option `-%c'\n", optopt);
						return -1;
					}
				    	else{
						printf ("Unknown option character `\\x%x'\n",optopt);
						return -1;
					}
				}
					
			}
		if (optind < argc)
			order = atoi(argv[optind]);
			if (order > 10){
				printf("The order given (%d) is too large... Using default maximum value: %d\n", order, MAX_ORDER);
				order = MAX_ORDER;
			}
		}

	printf("\n---------- Newton ----------\n\n");
	printf("Using %d threads\n", n_threads);
	printf("Image size is %d\n", image_size);
	printf("Order is %d\n", order);

	return 0;
} 

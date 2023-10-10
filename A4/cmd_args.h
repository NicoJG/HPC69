#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "global_vars.h"

static inline
void parse_cmd_args(int argc, char *argv[], short *n_its, float *diff_const) {
    int opt, val;

	if (argc > 1) {
		while((opt = getopt(argc, argv, "n:d:")) != -1) {
			switch(opt) {
				case 'n':
					val = atoi(optarg);
					if (val == 0) {
						printf("'%s' is not a valid integer... Using default number of iterations, %d.\n", optarg, DEFAULT_ITS);
						*n_its = DEFAULT_ITS;
						break;
					}
					*n_its = val;
					break;
				case 'l':
					val = atof(optarg);
					if (val == 0) {
						printf("Invalid argument for option -l. '%s' is not a valid float ... Using default diffusion constant, %d.\n", optarg, DEFAULT_DIFFUSION_CONSTANT);
						*diff_const = DEFAULT_DIFFUSION_CONSTANT;
						break;
					}
					*diff_const = val;
					break;

				case '?':
					if (optopt == 'n'){
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
    }
}

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "global_vars.h"

static inline
void parse_cmd_args(int argc, char *argv[]) {
	int opt, int_val;
	float float_val;

	if (argc > 1) {
		while((opt = getopt(argc, argv, "n:d:")) != -1) {
			switch(opt) {
				case 'n':
					int_val = atoi(optarg);
					if (int_val == 0) {
						printf("'%s' is not a valid integer... Using default number of iterations, %d.\n", optarg, DEFAULT_ITS);
						n_its = DEFAULT_ITS;
						break;
					}
					n_its = int_val;
					break;
				case 'd':
					float_val = atof(optarg);

					if (float_val == 0.0) {
						printf("Invalid argument for option -d. '%s' is not a valid diffusion constant ... Using default diffusion constant, %f.\n", optarg, DEFAULT_DIFFUSION_CONSTANT);
						diff_const = DEFAULT_DIFFUSION_CONSTANT;
						break;
					}
					diff_const = float_val;
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

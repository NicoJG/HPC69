#include <stdlib.h>
#include <stdio.h>

#include "cmd_args.h"

int 
main(
    int argc,
    char *argv[]
) {
    short n_its = 20;
    float diff_const = 0.02;

    parse_cmd_args(argc, argv, &n_its, &diff_const);

    printf("Computing %d iterations with diffusion constant %f.\n", n_its, diff_const);

    return 0;
    
}
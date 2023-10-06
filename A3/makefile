CC = gcc
CFLAGS = -O2 -march=native -g -lm

HEADERS = global_vars.h cmd_args.h computations.h write_ppm.h

.PHONY : all clean run

all : \
	newton

newton : newton.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

clean : 
	rm -f newton
	rm -f *.ppm
	rm -rf extracted
	rm -rf pictures
	rm -f A3_group69_submission.tar.gz

A3_group69_submission.tar.gz : makefile newton.c $(HEADERS)
	tar czf $@ $^

test_with_script : A3_group69_submission.tar.gz
	/home/hpc2023/newton_iteration/check_submission.jl $<
	rm -rf extracted
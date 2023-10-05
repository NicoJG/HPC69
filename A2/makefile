CC = gcc
CFLAGS = -O2 -march=native -g -lm -fopenmp

.PHONY : all clean test_with_script

all : \
	cell_distances 

cell_distances : \
	cell_distances.c \
	constants.h \
	read_file.h \
	compute_distance.h 
	$(CC) $(CFLAGS) -o $@ $<

clean : 
	rm -f cell_distances
	rm -f A2_group69_submission.tar.gz

A2_group69_submission.tar.gz : makefile \
	cell_distances.c \
	constants.h \
	read_file.h \
	compute_distance.h 
	tar czf $@ $^

test_with_script : A2_group69_submission.tar.gz
	/home/hpc2023/cell_distances/check_submission.jl $<
	rm -rf extracted

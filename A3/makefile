CC = gcc
CFLAGS = -O2 -march=native -g -lm

HEADERS = global_vars.h cmd_args.h computations.h 

.PHONY : all clean run

all : \
	exc_newton.exe \
	exc_write_ppm.exe

exc_newton.exe : newton.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

exc_write_ppm.exe : \
	write_ppm.c
	$(CC) $(CFLAGS) -o $@ $<
clean : 
	rm -f exc_cell_dist.exe exc_write_ppm.exe

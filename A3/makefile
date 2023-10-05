CC = gcc
CFLAGS = -O3 -march=native -g -lm

HEADERS = global_vars.h cmd_args.h computations.h write_ppm.h

.PHONY : all clean run

all : \
	exc_newton.exe

exc_newton.exe : newton.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

clean : 
	rm -f exc_newton.exe
	rm -f *.ppm
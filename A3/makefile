CC = gcc
CFLAGS = -O2 -march=native -g -lm

.PHONY : all clean run

all : \
	exc_newton.exe \
	exc_write_ppm.exe

exc_newton.exe : \
	newton.c \
	# header.h
	# ...
	$(CC) $(CFLAGS) -o $@ $<

exc_write_ppm.exe : \
	write_ppm.c
	$(CC) $(CFLAGS) -o $@ $<
clean : 
	rm -f exc_cell_dist.exe exc_write_ppm.exe

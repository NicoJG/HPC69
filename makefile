CC = gcc
CFLAGS = -O2 -march=native -g -lm

.PHONY : all clean run

all : \
	exc_cell_dist.exe

exc_cell_dist.exe : \
	cell_distances.c \
	constants.h \
	read_file.h \
	compute_distance.h 
	$(CC) $(CFLAGS) -o $@ $<

clean : 
	rm -f exc_cell_dist.exe

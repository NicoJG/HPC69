CC = gcc
CFLAGS = -march=native -g

.PHONY : all clean run

all : \
	read_cell_file

read_cell_file : \
	read_file.c 
	$(CC) $(CFLAGS) cell_distances.c read_file.c -o executable_cell_distances

clean : 
	rm -f read_cell_file	

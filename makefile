CC = gcc
CFLAGS = -march=native -g

.PHONY : all clean run

all : \
	read_cell_file

read_cell_file : \
	read_file.c 
	$(CC) $(CFLAGS) -o $@ $<

clean : 
	rm -f read_cell_file
	
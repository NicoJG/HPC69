CC = gcc
CFLAGS = -march=native -g

# List of source files
SRCS = cell_distances.c read_file.c

# Create a list of object files by replacing .c with .o
OBJS = $(patsubst %.c,%.o,$(SRCS))

.PHONY : all clean run

all : \
	exc_cell_dist

exc_cell_dist : $(OBJS) cell_distances.h
	$(CC) $(CFLAGS) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean : 
	rm -f exc_cell_dist
	rm -f $(OBJS)

CC = gcc
CFLAGS = -march=native -g -lm

# List of source files
SRCS = cell_distances.c read_file.c compute_distance.c test.c

# Create a list of object files by replacing .c with .o
OBJS = $(patsubst %.c,%.o,$(SRCS))

.PHONY : all clean run

all : \
	exc_cell_dist

exc_cell_dist.exe : $(OBJS) cell_distances.h
	$(CC) $(CFLAGS) -o $@ $^

test.exe : \
	test.o \
	compute_distance.o
	$(CC) $(CFLAGS) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean : 
	rm -f exc_cell_dist.exe
	rm -f $(OBJS)

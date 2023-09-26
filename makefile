CC = gcc
CFLAGS = -O2 -march=native -g -lm

# List of source files
SRCS = cell_distances.c read_file.c compute_distance.c

# Create a list of object files by replacing .c with .o
OBJS = $(patsubst %.c,%.o,$(SRCS))

.PHONY : all clean run

all : \
	exc_cell_dist.exe

exc_cell_dist.exe : $(OBJS) cell_distances.h
	$(CC) $(CFLAGS) -o $@ $^

test.exe : \
	test.o \
	compute_distance.o
	$(CC) $(CFLAGS) -o $@ $^

test.o : \
	test.c
	$(CC) $(CFLAGS) -c 

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean : 
	rm -f exc_cell_dist.exe test.exe
	rm -f $(OBJS)

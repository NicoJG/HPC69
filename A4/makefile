CC = gcc
CFLAGS = -O2 -march=native -g -lOpenCL

HEADERS = global_vars.h cmd_args.h read_file.h

.PHONY : all clean run

all : \
	diffusion

diffusion : diffusion.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

clean : 
	rm -f diffusion

run : diffusion
	./diffusion

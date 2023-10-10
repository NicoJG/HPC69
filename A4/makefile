CC = gcc
CFLAGS = -O2 -march=native -g -lm

HEADERS = global_vars.h cmd_args.h

.PHONY : all clean run

all : \
	diffusion

diffusion : diffusion.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

clean : 
	rm -f diffusion

run : diffusion
	./diffusion
CC = gcc
CFLAGS = -O2 -march=native -g -lOpenCL

HEADERS = global_vars.h cmd_args.h read_file.h

.PHONY : all clean run

all : \
	diffusion \
	A4_group69_submission.tar.gz

diffusion : diffusion.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

A4_group69_submission.tar.gz : makefile diffusion.c $(HEADERS) heat_calc.cl
	tar czf $@ $^

test_with_script : A4_group69_submission.tar.gz
	/home/hpc2023/diffusion_opencl/check_submission.jl $< --show-output 
	rm -rf extracted

clean : 
	rm -f diffusion
	rm -rf A4_group69_submission.tar.gz
	rm -rf extracted

run : diffusion
	./diffusion

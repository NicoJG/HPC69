CC = gcc
LIBRARIES = -L/usr/lib64/openmpi/lib -lm -lmpi -Wl,-rpath,/usr/lib64/openmpi/lib -Wl,--enable-new-dtags
CFLAGS  = -O2 -g -I. -I/usr/include/openmpi-x86_64 -pthread $(LIBRARIES)

HEADERS = cmd_args.h read_file.h global_vars.h

.PHONY : all clean run

all : \
	diffusion 

diffusion : diffusion.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

A5_group2_submission.tar.gz : makefile diffusion.c $(HEADERS)
	tar czf $@ $^

test_with_script : A5_group2_submission.tar.gz
	/home/hpc2023/diffusion_mpi/check_submission.jl $<
	rm -rf extracted

clean : 
	rm -f diffusion
	rm -rf A5_group2_submission.tar.gz
	rm -rf extracted

run : diffusion
	./diffusion

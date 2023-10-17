CC = gcc
LIBRARIES = -L/usr/lib64/openmpi/lib -lm -lmpi -Wl,-rpath,/usr/lib64/openmpi/lib -Wl,--enable-new-dtags
CFLAGS  = -O2 -I. -I/usr/include/openmpi-x86_64 -pthread $(LIBRARIES)

HEADERS = 

.PHONY : all clean run

all : \
	diffusion 

diffusion : diffusion.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

A5_group2_submission.tar.gz : makefile diffusion.c $(HEADERS)
	tar czf $@ $^

test_with_script : A4_group2_submission.tar.gz
	/home/hpc2023/diffusion_openmp/check_submission.jl $< --show-output 
	rm -rf extracted

clean : 
	rm -f diffusion
	rm -rf A5_group2_submission.tar.gz
	rm -rf extracted

run : diffusion
	./diffusion

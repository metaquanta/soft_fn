soft_fn : soft_fn.o
	cc -o soft_fn soft_fn.o

soft_fn.o : soft_fn.c stupidlayers.c
	cc -c soft_fn.c

clean : 
	rm soft_fn.o soft_fn

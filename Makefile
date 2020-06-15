soft_fn : soft_fn.o
	gcc -o soft_fn soft_fn.o

soft_fn.o : soft_fn.c stupidlayers.c
	gcc -O3 -fwhole-program -std=c99 -Wall -Wextra -pedantic -c soft_fn.c stupidlayers.c

clean : 
	-rm *.o 
	-rm soft_fn

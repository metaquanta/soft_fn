all : soft_fn toprow

soft_fn : soft_fn.o
	gcc -o soft_fn soft_fn.o

soft_fn.o : soft_fn.c
	gcc -Ofast -fwhole-program -std=c99 -Wall -Wextra -pedantic -c soft_fn.c

toprow : toprow.o
	gcc -o toprow toprow.o

toprow.o : toprow.c
	gcc -O2 -std=c99 -Wall -Wextra -pedantic -c toprow.c

clean :
	-rm *.o
	-rm soft_fn
	-rm toprow

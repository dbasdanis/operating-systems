find_roots_lib:
	cc -Wall -c roots.c
	ar -cvq libroots.a roots.o
	cc -o find_roots_lib find_roots_lib.c libroots.a

clean:	
	rm -f *.o *.a

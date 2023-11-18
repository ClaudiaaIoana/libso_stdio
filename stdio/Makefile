build: so_io.o
	gcc -shared so_io.o -o so_stdio.so
so_io.o:
	gcc -fPIC -c so_io.c -o so_io.o
clean:
	rm *.o main
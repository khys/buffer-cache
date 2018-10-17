bufcache: main.o getblk.o subr.o
	gcc -o bufcache main.o getblk.o subr.o

main.o: main.c buf.h
	gcc -c main.c

getblk.o: getblk.c buf.h
	gcc -c getblk.c

subr.o: subr.c buf.h
	gcc -c subr.c

clean:
	\rm bufcache *.o

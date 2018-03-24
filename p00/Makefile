all: output clean

output: testafila.o queue.o
	gcc testafila.o queue.o -o output

testafila.o: testafila.c
	gcc -c testafila.c

queue.o: queue.c queue.h
	gcc -c queue.c

clean:
	rm *.o

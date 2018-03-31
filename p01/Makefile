all: clean_out output clean

output: contexts.o 
	gcc contexts.o -o output

contexts.o: 
	gcc -c contexts.c

clean_out:
	rm -rf output

clean:
	rm -rf *.o

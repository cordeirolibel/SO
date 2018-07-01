all: clean_out output clean

output: pingpong-mqueue.o queue.o pingpong.o
	gcc -Wall -Wextra pingpong-mqueue.o queue.o pingpong.o -o output -lm

pingpong-mqueue.o: pingpong-mqueue.c
	gcc -c pingpong-mqueue.c

pingpong.o: pingpong.c pingpong.h datatypes.h
	gcc -c pingpong.c

queue.o: ../p00/queue.c ../p00/queue.h
	gcc -c ../p00/queue.c

clean_out:
	rm -rf output
	rm -rf *.o

clean:
	rm -rf *.o

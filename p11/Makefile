all: clean_out output clean

output: pingpong-barrier.o queue.o pingpong.o
	gcc -Wall -Wextra pingpong-barrier.o queue.o pingpong.o -o output

pingpong-barrier.o: pingpong-barrier.c
	gcc -c pingpong-barrier.c

pingpong.o: pingpong.c pingpong.h datatypes.h
	gcc -c pingpong.c

queue.o: ../p00/queue.c ../p00/queue.h
	gcc -c ../p00/queue.c

clean_out:
	rm -rf output
	rm -rf *.o

clean:
	rm -rf *.o

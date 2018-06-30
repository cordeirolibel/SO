all: clean_out output clean

output: pingpong-prodcons.o queue.o pingpong.o
	gcc -Wall -Wextra pingpong-prodcons.o queue.o pingpong.o -o output

pingpong-racecond.o: pingpong-racecond.c
	gcc -c pingpong-racecond.c

pingpong-semaphore.o: pingpong-semaphore.c
	gcc -c pingpong-semaphore.c
	
pingpong-prodcons.o: pingpong-prodcons.c
	gcc -c pingpong-prodcons.c

pingpong.o: pingpong.c pingpong.h datatypes.h
	gcc -c pingpong.c

queue.o: ../p00/queue.c ../p00/queue.h
	gcc -c ../p00/queue.c

clean_out:
	rm -rf output
	rm -rf *.o

clean:
	rm -rf *.o

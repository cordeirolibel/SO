all: clean_out output clean

output: pingpong-sleep.o queue.o pingpong.o
	gcc -Wall -Wextra pingpong-sleep.o queue.o pingpong.o -o output

pingpong-sleep.o: pingpong-sleep.c
	gcc -c pingpong-sleep.c

pingpong.o: pingpong.c pingpong.h datatypes.h
	gcc -c pingpong.c

queue.o: ../p00/queue.c ../p00/queue.h
	gcc -c ../p00/queue.c

clean_out:
	rm -rf output
	rm -rf *.o

clean:
	rm -rf *.o

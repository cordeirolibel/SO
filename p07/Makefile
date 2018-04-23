all: clean_out output clean

output: pingpong-maintask.o queue.o pingpong.o
	gcc -Wall -Wextra pingpong-maintask.o queue.o pingpong.o -o output

pingpong-maintask.o: pingpong-maintask.c
	gcc -c pingpong-maintask.c

pingpong.o: pingpong.c pingpong.h datatypes.h
	gcc -c pingpong.c

queue.o: ../p00/queue.c ../p00/queue.h
	gcc -c ../p00/queue.c

clean_out:
	rm -rf output
	rm -rf *.o

clean:
	rm -rf *.o

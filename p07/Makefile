all: clean_out output clean

output: pingpong-contab.o pingpong-contab-prio.o queue.o pingpong.o
	gcc -Wall -Wextra pingpong-contab-prio.o queue.o pingpong.o -o output

pingpong-contab.o: pingpong-contab.c
	gcc -c pingpong-contab.c

pingpong-contab-prio.o: pingpong-contab-prio.c
	gcc -c pingpong-contab-prio.c

pingpong.o: pingpong.c pingpong.h datatypes.h
	gcc -c pingpong.c

queue.o: ../p00/queue.c ../p00/queue.h
	gcc -c ../p00/queue.c

clean_out:
	rm -rf output
	rm -rf *.o

clean:
	rm -rf *.o

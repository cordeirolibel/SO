all: clean_out output clean

output: pingpong-scheduler.o queue.o pingpong.o
	gcc pingpong-scheduler.o queue.o pingpong.o -o output

pingpong-scheduler.o: pingpong-scheduler.c
	gcc -c pingpong-scheduler.c

pingpong.o: pingpong.c pingpong.h datatypes.h
	gcc -c pingpong.c

queue.o: ../p00/queue.c ../p00/queue.h
	gcc -c ../p00/queue.c
	echo ola

clean_out:
	rm -rf output
	rm -rf *.o

clean:
	rm -rf *.o

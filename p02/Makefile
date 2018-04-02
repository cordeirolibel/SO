all: clean_out output clean

output: pingpong-tasks1.o pingpong-tasks2.o pingpong-tasks3.o pingpong.o
	gcc pingpong-tasks3.o pingpong.o -o output

pingpong-tasks1.o: pingpong-tasks1.c
	gcc -c pingpong-tasks1.c

pingpong-tasks2.o: pingpong-tasks2.c
	gcc -c pingpong-tasks2.c

pingpong-tasks3.o: pingpong-tasks3.c
	gcc -c pingpong-tasks3.c

pingpong.o: pingpong.c pingpong.h datatypes.h
	gcc -c pingpong.c

clean_out:
	rm -rf output
	rm -rf *.o

clean:
	rm -rf *.o

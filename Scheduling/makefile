all: main

run: main
	./main

main: Scheduling.o lists.o global.o
	gcc -o main Scheduling.o lists.o global.o

Scheduling.o:
	gcc -c Scheduling.c

lists.o:
	gcc -c lists.c

global.o:
	gcc -c global.c

clean:
	rm -f main Scheduling.o lists.o global.o

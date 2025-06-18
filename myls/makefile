all: main

run: main
	./myls

main: main.o
	gcc -o myls main.o

main.o: main.c
	gcc -c main.c
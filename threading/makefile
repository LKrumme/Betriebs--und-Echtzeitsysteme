all: main

run: main
	./main

main: main.o miniz.o
	gcc -o main main.o miniz.o

main.o: main.c
	gcc -c main.c

miniz.o: miniz.c
	gcc -c miniz.c

clean:
	rm -f main main.o miniz.o
quash: main.o
	gcc main.o -o quash

main.o: main.c
	gcc -c -g main.c

clean:
	rm -f *.o quash

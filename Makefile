main: msh.c
	gcc -Wall msh.c -o msh

debug: msh.c
	gcc -Wall msh.c -o debug -g

clean:
	-rm -rf *.o msh debug*
main: msh.c
	gcc -Wall msh.c -o msh

clean:
	-rm -rf *.o msh
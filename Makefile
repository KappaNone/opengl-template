main: main.o
	gcc -Wall -Wextra main.o -o main -lGL -lglfw -lGLEW 

main.o: main.c shaders/shader.vert shaders/blink.frag
	gcc -Wall -Wextra -c main.c

clean:
	rm -rf *.o main

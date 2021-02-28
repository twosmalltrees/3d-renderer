build:
	gcc -Wall -std=c99 -lSDL2 src/*.c -o bin/renderer

run:
	./bin/renderer

clean:
	rm ./bin/renderer
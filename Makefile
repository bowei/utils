all: bin null-server

bin:
	mkdir bin

null-server: null-server.c
	gcc -std=gnu99 -O0 -Wall -g -o bin/null-server.debug null-server.c
	gcc -std=gnu99 -O2 -Wall -g -o bin/null-server null-server.c

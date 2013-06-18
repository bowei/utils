BINS := \
	bin/null-server

all: bin $(BINS)

bin:
	mkdir bin

bin/null-server: null-server.c
	gcc -std=gnu99 -O0 -Wall -g -o bin/null-server.debug null-server.c
	gcc -std=gnu99 -O2 -Wall -g -o bin/null-server null-server.c

clean:
	rm -f $(BINS)

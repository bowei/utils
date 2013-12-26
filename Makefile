BINS := \
	bin/null-server \
	bin/ipc-rtt

all: bin $(BINS)

bin:
	mkdir bin

bin/null-server: null-server.c
	gcc -std=gnu99 -O0 -Wall -g -o bin/null-server.debug null-server.c
	gcc -std=gnu99 -O2 -Wall -g -o bin/null-server null-server.c

bin/ipc-rtt: ipc-rtt.c
	gcc -std=gnu99 -O0 -Wall -g -o bin/ipc-rtt.debug ipc-rtt.c
	gcc -std=gnu99 -O2 -Wall -g -o bin/ipc-rtt ipc-rtt.c

clean:
	rm -f $(BINS)

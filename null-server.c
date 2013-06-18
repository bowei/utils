#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

// For getopt(3)
extern char *optarg;
extern int optind, opterr, optopt;

// globals
static int g_port = -1;
static int g_listen_sock = -1;

void print_usage();
int parse_args(int argc, char* argv[]);
void pcheck(int ret);

int
main(int argc, char* argv[])
{
  if (! parse_args(argc, argv)) {
    print_usage();
    exit(1);
  }

  if (g_port <= 0) {
    fprintf(stderr, "You must specify a valid port (-p <port>)\n");
    exit(1);
  } else {
    fprintf(stderr, "Using port %d\n", g_port);
  }

  g_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
  pcheck(g_listen_sock);

  struct sockaddr_in addr_in;
  memset(&addr_in, 0, sizeof(addr_in));
  addr_in.sin_port = g_port;
  addr_in.sin_port = htons(addr_in.sin_port);

  int ret = bind(g_listen_sock, (struct sockaddr*) &addr_in, sizeof(addr_in));
  pcheck(ret);

  ret = listen(g_listen_sock, 1000);
  pcheck(ret);

  while (1) {
    struct sockaddr_storage client_addr;
    socklen_t socklen = sizeof(client_addr);

    int sock = accept(g_listen_sock, (struct sockaddr*) &client_addr, &socklen);
    // fprintf(stderr, "accepted\n");
    close(sock);
  }

  return 0;
}

void
print_usage()
{
  fprintf(stdout, "Usage: -p <port>\n");
}

int
parse_args(int argc, char* argv[])
{
  int opt;
  while ((opt = getopt(argc, argv, "hp:")) != -1) {
    switch (opt) {
      case 'p':
        g_port = atoi(optarg);
        break;
      case 'h':
        print_usage();
        exit(0);
      default:
        print_usage();
        return 0;
    }
  }

  return 1;
}

void
pcheck(int ret)
{
  if (ret < 0) {
    fprintf(stderr, "Failed: %s", strerror(errno));
    abort();
  }
}

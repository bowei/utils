// Run a simple TCP client/server and measures the latency in the
// reply. This is basically as stripped down as you can get.
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

typedef struct {
  int mode; // 0 = server, 1 = client.
  int server_port;
} Options;
Options g_options = { 0, 9999 };

typedef struct {
  int epoll_fd;
  int listen_fd;
} ServerState;
ServerState g_server = {};

#define PCHECK(x)                                               \
  if ((x) < 0) {                                                \
    fprintf(stderr, #x " failed: %s\n", strerror(errno));       \
    abort();                                                    \
  }
#define CHECK(x)                                                \
  if (!(x)) {                                                    \
    fprintf(stderr, #x " failed: %s\n", strerror(errno));       \
    abort();                                                    \
  }

int parse_command_line();
void print_usage();
int64_t now_in_usecs();
void run_server();
void run_client();

int
parse_command_line(int argc, char* argv[])
{
  if (argc > 1 && strcmp(argv[1], "-c") == 0) {
    g_options.mode = 1;
  }

  return 1;
}

void
print_usage()
{
}

int64_t
now_in_usecs()
{
  struct timeval tv;
  PCHECK(gettimeofday(&tv, NULL));
  return (tv.tv_sec * 1000000LL + tv.tv_usec);
}

void
run_server()
{
  memset(&g_server, 0, sizeof(g_server));
  g_server.epoll_fd = epoll_create(1);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  PCHECK(fd);

  struct sockaddr_in addr_in;
  memset(&addr_in, 0, sizeof(addr_in));
  addr_in.sin_port = g_options.server_port;
  addr_in.sin_port = htons(addr_in.sin_port);

  int reuseaddr = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
             &reuseaddr, sizeof(reuseaddr));

  int ret = bind(fd, (struct sockaddr*) &addr_in, sizeof(addr_in));
  PCHECK(ret);

  ret = listen(fd, 1000);
  PCHECK(ret);

  g_server.listen_fd = fd;

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = g_server.listen_fd;
  PCHECK(epoll_ctl(g_server.epoll_fd, EPOLL_CTL_ADD,
                   g_server.listen_fd, &ev));

#define MAX_EVENTS 100
  struct epoll_event active_events[MAX_EVENTS];

  for (;;) {
    int num_events = epoll_wait(
      g_server.epoll_fd, active_events, MAX_EVENTS, -1);
    PCHECK(num_events);
    for (int i=0; i<num_events; ++i) {
      // printf("got event for fd=%d\n", active_events[i].data.fd);

      fd = active_events[i].data.fd;
      if (fd == g_server.listen_fd) {
        struct sockaddr_storage client_addr;
        socklen_t socklen = sizeof(client_addr);

        int sock;
        /*
        PCHECK(sock = accept4(g_server.listen_fd,
                              (struct sockaddr*)&client_addr,
                              &socklen, SOCK_NONBLOCK));
        */
        PCHECK(sock = accept4(g_server.listen_fd,
                              (struct sockaddr*)&client_addr,
                              &socklen, 0));
        ev.events = EPOLLIN;
        ev.data.fd = sock;
        PCHECK(epoll_ctl(g_server.epoll_fd, EPOLL_CTL_ADD, sock, &ev));
      } else {
        char c;
        int cc = read(fd, &c, 1);
        if (cc == 0) {
          PCHECK(close(fd));
        } else {
          PCHECK(write(fd, &c, 1));
        }
      }
    }
  }
}

void
run_client()
{
  printf("conn\tsend\treply\ttotal\n");
  for (;;) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    PCHECK(fd);

    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = g_options.server_port;
    addr_in.sin_port = htons(addr_in.sin_port);

    int64_t start_us, connect_us, sent_us, reply_us;

    start_us = now_in_usecs();
    PCHECK(connect(fd, (struct sockaddr*)&addr_in, sizeof(addr_in)));
    connect_us = now_in_usecs();
    char c = 'a';
    CHECK(write(fd, &c, 1) == 1);
    sent_us = now_in_usecs();
    CHECK(read(fd, &c, 1) == 1);
    reply_us = now_in_usecs();
    close(fd);

    printf("%ld\t%ld\t%ld\t%ld\n",
           connect_us - start_us,
           sent_us - connect_us,
           reply_us - sent_us,
           reply_us - start_us);

    usleep(1000000);
  }
}

int
main(int argc, char* argv[])
{
  if (! parse_command_line(argc, argv)) {
    print_usage();
    exit(1);
  }

  if (g_options.mode == 0) {
    run_server();
  } else if (g_options.mode == 1) {
    run_client();
  } else { abort(); }
}

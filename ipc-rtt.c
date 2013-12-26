// Run a simple TCP client/server and measures the latency in the
// reply. This is basically as stripped down as you can get.
//
// This is benchmark code, not for production.
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
  int msg_size;
  int interval_us;
  int samples;
} Options;
static Options g_options = { 0, 9999, 1, 1000, 10000 };

typedef struct {
  int epoll_fd;
  int listen_fd;
} ServerState;
static ServerState g_server = {};

typedef struct {
  int64_t* rtt;
  int rtt_size;
} Stats;
Stats g_stats = {};

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

int int64_compare(const void* a, const void* b) {
  int64_t av = *((const int64_t*) a);
  int64_t bv = *((const int64_t*) b);
  return av - bv;
}

int parse_command_line();
void print_usage();
int64_t now_in_usecs();
void run_server();
void run_client();

int
parse_command_line(int argc, char* argv[])
{
  int opt;
  while ((opt = getopt(argc, argv, "hcp:m:i:")) != -1) {
    switch (opt) {
      case 'h':
        print_usage();
        return 0;
        break;
      case 'c':
        g_options.mode = 1;
        break;
      case 'p':
        g_options.server_port = atoi(optarg);
        break;
      case 'm':
        g_options.msg_size = atoi(optarg);
        break;
      case 'i':
        g_options.interval_us = atoi(optarg);
        break;
      default:
        print_usage();
        return 0;
    }
  }

  return 1;
}

void
print_usage()
{
  // XXX
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
#define MAX_BUF 10*1024*1024
  char* buf = malloc(MAX_BUF);

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
        int cc;
        int size;
        PCHECK(cc = read(fd, &size, sizeof(size)));

        if (cc == 0) {
          PCHECK(close(fd));
        } else {
          int bytes_read = 0;
          while (bytes_read < size) {
            PCHECK(cc = read(fd, buf, size - bytes_read));
            bytes_read += cc;
            PCHECK(write(fd, buf, cc));
          }
        }
      }
    }
  }
}

void
run_client()
{
  // printf("conn\tsend\treply\ttotal\n");
  char* buf = malloc(g_options.msg_size);
  memset(buf, 1, g_options.msg_size);

  for (int i=0; i<g_options.samples; ++i) {
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

    PCHECK(write(fd, &g_options.msg_size, sizeof(g_options.msg_size)));
    CHECK(write(fd, buf, g_options.msg_size) == g_options.msg_size);
    sent_us = now_in_usecs();
    CHECK(read(fd, buf, g_options.msg_size) == g_options.msg_size);
    reply_us = now_in_usecs();
    close(fd);
/*    printf("%ld\t%ld\t%ld\t%ld\n",
           connect_us - start_us,
           sent_us - connect_us,
           reply_us - sent_us,
           reply_us - start_us);
*/
    (void) start_us;
    (void) connect_us;

    g_stats.rtt[i] = reply_us - sent_us;
    usleep(g_options.interval_us);
  }

  qsort(g_stats.rtt, g_options.samples, sizeof(g_stats.rtt[0]),
        int64_compare);

  int p50_index = g_options.samples*0.5;
  int p95_index = g_options.samples*0.95;
  int p99_index = g_options.samples*0.99;
  printf("min\t50%%\t95%%\t99%%\tmax\n");
  printf("%ld\t%ld\t%ld\t%ld\t%ld\n",
         g_stats.rtt[0],
         g_stats.rtt[p50_index],
         g_stats.rtt[p95_index],
         g_stats.rtt[p99_index],
         g_stats.rtt[g_options.samples-1]);
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
    g_stats.rtt_size = g_options.samples;
    g_stats.rtt = calloc(sizeof(int64_t), g_stats.rtt_size);
    run_client();
  } else { abort(); }
}

#include "falcon/meas/broadcast_slave.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>

void errno_abort(const char* header) {
  perror(header);
  exit(EXIT_FAILURE);
}

broadcast_slave_t* broadcast_slave_init(uint16_t port) {
  broadcast_slave_t* h = calloc(1, sizeof (broadcast_slave_t));

  int trueflag = 1;
  if ((h->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    errno_abort("socket");
  }

  if (setsockopt(h->fd, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(trueflag)) < 0) {
    errno_abort("setsockopt");
  }

  memset(&h->recv_addr, 0, sizeof(h->recv_addr));
  h->recv_addr.sin_family = AF_INET;
  h->recv_addr.sin_port = (in_port_t)htons(port);
  h->recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(h->fd, (struct sockaddr*)&h->recv_addr, sizeof(h->recv_addr)) < 0) {
    errno_abort("bind");
  }

  h->send_ready = 0;
  memset(&h->send_addr, 0, sizeof(h->send_addr));

  return h;
}

void broadcast_slave_destroy(broadcast_slave_t* h) {
  if(h) {
    close(h->fd);
    h->fd = 0;
    free(h);
  }
}

size_t broadcast_slave_receive(broadcast_slave_t* h, char *msg, size_t len) {
  socklen_t other_addr_len = sizeof (h->send_addr);
  ssize_t recv_len = 0;
  recv_len = recvfrom(h->fd, msg, len, 0, (struct sockaddr*)&h->send_addr, &other_addr_len);
  if (recv_len < 0) {
    errno_abort("recv");
    recv_len = 0;
  }
  else {
    h->send_ready = 1;
  }

  return (size_t)recv_len;
}

int32_t broadcast_slave_reply(broadcast_slave_t* h, const char *msg, size_t len) {
  if(h->send_ready) {
    if (sendto(h->fd, msg, len, 0, (struct sockaddr*)&h->send_addr, sizeof(h->send_addr)) < 0) {
      errno_abort("send");
      return -1;
    }
  }
  return 0;
}

#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

#include <netinet/ip.h>

typedef struct {
  int fd;
  int send_ready;
  struct sockaddr_in recv_addr;
  struct sockaddr_in send_addr;
} broadcast_slave_t;

broadcast_slave_t* broadcast_slave_init(uint16_t port);
void broadcast_slave_destroy(broadcast_slave_t* h);
size_t broadcast_slave_receive(broadcast_slave_t* h, char *msg, size_t len);
int32_t broadcast_slave_reply(broadcast_slave_t* h, const char* msg, size_t len);

#ifdef __cplusplus
}
#endif

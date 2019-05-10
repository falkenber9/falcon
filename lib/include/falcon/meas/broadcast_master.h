#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

#include <netinet/ip.h>

typedef struct {
  int fd;
  struct sockaddr_in recv_addr;
  struct sockaddr_in send_addr;
} broadcast_master_t;

broadcast_master_t* broadcast_master_init(const char ip[], uint16_t port);
void broadcast_master_destroy(broadcast_master_t* h);
size_t broadcast_master_receive(broadcast_master_t* h, char *msg, size_t len);
int32_t broadcast_master_send(broadcast_master_t* h, const char* msg, size_t len);


int init_example(int argc, char* argv[]);

#ifdef __cplusplus
}
#endif

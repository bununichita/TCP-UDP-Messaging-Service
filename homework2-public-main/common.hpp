#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <stddef.h>
#include <stdint.h>

int send_all(int sockfd, void *buff, size_t len);
int recv_all(int sockfd, void *buff, size_t len);

/* Dimensiunea maxima a mesajului */
#define MSG_MAXSIZE 1552
#define TOPIC_MAXSIZE 50
#define MAX_ID_SIZE 10

struct chat_packet {
  char client[MAX_ID_SIZE + 1];
  uint16_t len;
  char message[MSG_MAXSIZE + 1];
};

#endif

#ifndef _HELPERS_HPP
#define _HELPERS_HPP 1

#include <stdio.h>
#include <stdlib.h>

#define MAX_CLIENT_ID_LENGTH 10
#define MAX_UDP_MSG_SIZE 1552
#define MAX_UDP_PAYLOAD_SIZE 1501
#define MAX_TOPIC_NAME_LENGTH 50

struct udp_msg {
	char topic[MAX_TOPIC_NAME_LENGTH];
	uint8_t type;
	char data[MAX_UDP_PAYLOAD_SIZE];
};

struct type_int {
	uint8_t sign;
	uint8_t num[4];
};

struct type_short_real {
	uint16_t num;
};

struct type_float {
	uint8_t sign;
	uint8_t num[4];
	uint8_t pow;
};

struct type_string {
	char str[MAX_UDP_PAYLOAD_SIZE];
};

#define DIE(assertion, call_description)                                       \
	do {                                                                         \
		if (assertion) {                                                           \
		fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
		perror(call_description);                                                \
		exit(EXIT_FAILURE);                                                      \
		}                                                                          \
	} while (0)

#endif

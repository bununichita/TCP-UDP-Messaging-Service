#include <iostream>
#include <vector>
#include <iomanip>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <cctype>
#include <poll.h>
#include <cmath>

#include "./helpers.hpp"
#include "./common.hpp"

using namespace std;

struct pollfd fds[2];
int pollfd_count = 2;

void run_client(int sockfd, char *client_id) {
	char buf[MSG_MAXSIZE + 1];
	memset(buf, 0, MSG_MAXSIZE + 1);

	struct tcp_msg sent_packet;
	// struct tcp_msg recv_packet;

	fds[0].fd = sockfd;
	fds[0].events = POLLIN;
	fds[1].fd = STDIN_FILENO;
	fds[1].events = POLLIN;

	while (true) {
		int rc = poll(fds, 2, -1);
		if (rc < 0) {
		perror("poll");
		break;
		}

		if (fds[0].revents & POLLIN) {
			size_t udpDataSize;
			rc = recv_all(sockfd, &udpDataSize, sizeof(udpDataSize));
			// cout << "UDP data size: " << udpDataSize << endl;
			DIE(rc < 0, "recv_all");

			if (rc == 0) {
				break;
			}

			// udp_msg *recv_packet;
			udp_msg msg;
			rc = recv_all(sockfd, &msg, 51 + udpDataSize);
			DIE(rc < 0, "recv_all");
			// recv_packet.message[1551] = '\0';
			if (msg.type == 0) {
				type_int *msg_int = (type_int *)msg.data;
				if (ntohl(*(unsigned int *)msg_int->num) == 0)
					cout << msg.topic << " - INT - " << 0 << endl;
				else if (msg_int->sign == 0)
					cout << msg.topic << " - INT - " << ntohl(*(unsigned int *)msg_int->num) << endl;
				else
					cout << msg.topic << " - INT - -" << ntohl(*(unsigned int *)msg_int->num) << endl;
			} else if (msg.type == 1) {
				type_short_real *msg_short_real = (type_short_real *)msg.data;
				cout << msg.topic << " - SHORT_REAL - " << fixed << setprecision(2) << (double)(ntohs(msg_short_real->num)) / 100 << endl;
			} else if (msg.type == 2) {
				type_float *msg_float = (type_float *)msg.data;
				uint32_t num = ntohl(*(unsigned int *)msg_float->num);
				double num_double = (double)num / (double)(pow(10, msg_float->pow));
				if (num_double == 0)
					cout << msg.topic << " - FLOAT - " << 0 << endl;
				else if (msg_float->sign == 0)
					cout << msg.topic << " - FLOAT - " << fixed << setprecision(msg_float->pow) << num_double << endl;
				else
					cout << msg.topic << " - FLOAT - -" << fixed << setprecision(msg_float->pow) << num_double << endl;
			} else if (msg.type == 3) {
				type_string *msg_string = (type_string *)msg.data;
				cout << msg.topic << " - STRING - " << msg_string->str << endl;
			} else {
				DIE(true, "Invalid message type");
			}
		}

		if (fds[1].revents & POLLIN) {
			if (fgets(buf, sizeof(buf), stdin) && !isspace(buf[0])) {
				if (!strcmp(buf, "exit\n")) {
					tcp_msg packet;
					strcpy(packet.client, client_id);
					uint16_t *len = (uint16_t *)packet.len;
					*len = (uint16_t)(strlen(buf) + 1);
					// packet.len = strlen(buf) + 1;
					strcpy(packet.message, buf);
					send_all(sockfd, &packet, sizeof(packet));
					break;
				}
				if (!strncmp(buf, "subscribe", 9)) {
					tcp_msg packet;
					memset(&packet, 0, sizeof(packet));
					strcpy(packet.client, client_id);
					buf[strlen(buf)] = '\0';
					uint16_t *len = (uint16_t *)packet.len;
					*len = (uint16_t)(strlen(buf) + 1);
					// packet.len = strlen(buf) + 1;
					strcpy(packet.message, buf);
					send_all(sockfd, &packet, sizeof(packet));

					string topic = buf;
					topic = topic.substr(10, topic.size() - 11);
					cout << "Subscribed to topic " << topic << endl;
					continue;
				}
				if (!strncmp(buf, "unsubscribe", 11)) {
					tcp_msg packet;
					strcpy(packet.client, client_id);
					uint16_t *len = (uint16_t *)packet.len;
					*len = (uint16_t)(strlen(buf) + 1);
					// packet.len = strlen(buf) + 1;
					strcpy(packet.message, buf);
					send_all(sockfd, &packet, sizeof(packet));
					string topic = buf;
					topic = topic.substr(12, topic.size() - 13);
					cout << "Unsubscribed from topic " << topic << endl;
					continue;
				}
				// sent_packet.len = strlen(buf) + 1;

				// strcpy(sent_packet.message, buf);
				// send_all(sockfd, &sent_packet, sizeof(sent_packet));
			}
		}
	}
}

int main(int argc, char *argv[]) {
	DIE(argc != 4, "Usage: ./subscriber <IP_SERVER> <PORT_SERVER>");
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	uint16_t port;
	int rc = sscanf(argv[3], "%hu", &port);
	DIE(rc != 1, "Given port is invalid");
	// Obtain a TCP socket for connecting to the server
	const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");
	// Complete the serv_addr address, address family and port for connection
	struct sockaddr_in serv_addr;
	socklen_t socket_len = sizeof(struct sockaddr_in);
	memset(&serv_addr, 0, socket_len);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
	DIE(rc <= 0, "inet_pton");
	// Connect to server
	rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	DIE(rc < 0, "connect");
	// Send the client ID to the server
	char* client_id = argv[1];
	send_all(sockfd, argv[1], MAX_CLIENT_ID_LENGTH);
	run_client(sockfd, argv[1]);
	// Close the connection
	close(sockfd);
	return 0;
}

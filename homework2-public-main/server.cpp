#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iomanip>
#include <cmath>
#include <regex>
#include <set>

#include "helpers.hpp"
#include "common.hpp"


#define START_CLIENTS 3

using namespace std;


struct subscribed_topic {
	std::string clientID;
	std::vector<std::string> topics;
};

struct TCPClient {
	std::string id;
	std::string ip;
	int port;
	int fd;
};




std::map<std::string, set<std::string>> idToTopics;


// Define a map to store the TCP clients
std::vector<TCPClient> tcpClients;


void removeIdTopic(std::string id, std::string topic) {
	for (long unsigned i = 0; i < idToTopics[id].size(); i++) {
		if (idToTopics[id].count(topic)) {
			idToTopics[id].erase(topic);
			break;
		}
	}
}


void run_chat_multi_server(int tcpfd, int udpfd) {

  vector<pollfd> poll_fds(3);

  int num_sockets = 3;
  int rc;

  tcp_msg received_packet;

  // Setam socket-ul tcpfd pentru ascultare
  rc = listen(tcpfd, num_sockets);
  DIE(rc < 0, "listen");

  // Adaugam noul file descriptor (socketul pe care se asculta conexiuni) in
  // multimea poll_fds
  poll_fds[0].fd = tcpfd;
  poll_fds[0].events = POLLIN;
  poll_fds[1].fd = STDIN_FILENO;
  poll_fds[1].events = POLLIN;
  poll_fds[2].fd = udpfd;
  poll_fds[2].events = POLLIN;

	while (1) {
		// Asteptam sa primim ceva pe unul dintre cei num_sockets socketi
		rc = poll(poll_fds.data(), num_sockets, -1);
		DIE(rc < 0, "poll");

		for (int i = 0; i < num_sockets; i++) {
			// std::cout << "intra in for" << std::endl;
			if (poll_fds[i].revents & POLLIN) {
				// std::cout << i << std::endl;
				if (poll_fds[i].fd == STDIN_FILENO) {
					// std::cout << "intra in stdin" << std::endl;
					char input[256];
					std::cin >> input;
					input[strlen(input)] = '\0'; // remove newline character

					if (strncmp(input, "exit", 4) == 0) {
						// Close the router
						for (auto fd : poll_fds) {
							close(fd.fd);
						}
						return;
					}
				} else if (poll_fds[i].fd == tcpfd) {
					// Am primit o cerere de conexiune pe socketul de listen, pe care
					// o acceptam
					struct sockaddr_in cli_addr;
					socklen_t cli_len = sizeof(cli_addr);
					const int newsockfd =
						accept(tcpfd, (struct sockaddr *)&cli_addr, &cli_len);
					DIE(newsockfd < 0, "accept");

					// Adaugam noul socket intors de accept() la multimea descriptorilor
					// de citire
					poll_fds.push_back(pollfd());
					poll_fds[num_sockets].fd = newsockfd;
					poll_fds[num_sockets].events = POLLIN;
					num_sockets++;

					// Receive client ID from subscriber
					char clientID[MAX_CLIENT_ID_LENGTH];
					int rc = recv_all(newsockfd, clientID, MAX_CLIENT_ID_LENGTH);
					DIE(rc < 0, "recv");
					clientID[rc] = '\0';

					// Check if a client with the same name already exists
					bool clientExists = false;
					for (const auto& client : tcpClients) {
						if (client.id == clientID) {
							clientExists = true;
							break;
						}
					}

					if (clientExists) {
						std::cout << "Client " << clientID << " already connected." << std::endl;
						close(newsockfd);
						poll_fds.erase(poll_fds.begin() + num_sockets - 1);
						num_sockets--;
						// poll_fds.pop_back();
					} else {
						// Create a TCPClient object and add it to the map
						TCPClient newClient;
						newClient.id = std::string(clientID);
						newClient.ip = inet_ntoa(cli_addr.sin_addr);
						newClient.port = ntohs(cli_addr.sin_port);
						newClient.fd = newsockfd;

						tcpClients.push_back(newClient);

						std::cout << "New client " << newClient.id << " connected from " << newClient.ip << ":" << newClient.port << std::endl;
					}
				} else if (poll_fds[i].fd == udpfd) {
					// Am primit date pe socketul udp, asa ca le receptionam
					// unsigned char buf[1600];
					// udp_msg udpMsg;
					udp_msg msg;					 
					struct sockaddr_in cli_addr;
					socklen_t cli_len = sizeof(cli_addr);
					rc = recvfrom(udpfd, &msg, MAX_UDP_MSG_SIZE, 0, (struct sockaddr *)&cli_addr, &cli_len);
					DIE(rc < 0, "recvfrom");

					uint8_t msgType = msg.type;
					size_t udpDataLen = rc - 51;
					// if (msgType == 0) {
					// 	udpDataLen = sizeof(uint32_t) + 1;
					// } else if (msgType == 1) {
					// 	udpDataLen = sizeof(uint16_t);
					// } else if (msgType == 2) {
					// 	udpDataLen = sizeof(uint32_t) + 2;
					// } else if (msgType == 3) {
					// 	udpDataLen = strlen(msg->data) + 1;
					// } else {
					// 	// Invalid message - Ignore
					// 	continue;
					// }

					for (const auto& client : tcpClients) {
						for (const auto& topic : idToTopics[client.id]) {
							if (regex_match(msg.topic, regex(topic))) {

								int rc = send_all(client.fd, &udpDataLen, sizeof(udpDataLen));
								DIE(rc < 0, "send");
								// Send the message to the client
								// tcp_msg packet;
								// memcpy(packet.client, client.id.c_str(), MAX_CLIENT_ID_LENGTH);
								// memcpy(packet.message, msg, MAX_UDP_MSG_SIZE);
								// uint16_t *len = (uint16_t *)packet.len;
								// *len = MAX_UDP_MSG_SIZE;
								// packet.len = MAX_UDP_MSG_SIZE;
								rc = send_all(client.fd, &msg, 51 + udpDataLen);
								DIE(rc < 0, "send");
								break;
							}
						}
					}
				} else if (poll_fds[i].fd > 0){
					// Am primit date pe unul din socketii de client, asa ca le receptionam
					int rc = recv_all(poll_fds[i].fd, &received_packet,
										sizeof(received_packet));
					DIE(rc < 0, "recv");

					if (rc == 0 || strstr(received_packet.message, "exit")) {
						std::cout << "Client " << received_packet.client << " disconnected." << std::endl;
						// Erase the tcpClients entry of the current client
						for (auto it = tcpClients.begin(); it != tcpClients.end(); it++) {
							if (!strcmp(it->id.c_str(), received_packet.client)) {
								tcpClients.erase(it);
								break;
							}
						}
						poll_fds[i].revents = 0;
						close(poll_fds[i].fd);
						poll_fds[i].fd = -1;

						// Scoatem din multimea de citire socketul inchis
						for (int j = i; j < num_sockets - 1; j++) {
							poll_fds[j] = poll_fds[j + 1];
						}

						num_sockets--;
						i--;
					} else {
						if (!strncmp(received_packet.message, "subscribe", 9)) {
							std::string topic = received_packet.message;
							topic = topic.substr(10, topic.size() - 11);

							std::string regexStr = topic;
							regexStr = regex_replace(regexStr, regex("\\*"), ".*");
    						regexStr = std::regex_replace(regexStr, std::regex("\\+"), "[^/]+");
							idToTopics[received_packet.client].insert(regexStr);
							continue;
						} else if (!strncmp(received_packet.message, "unsubscribe", 11)) {
							std::string topic = received_packet.message;
							topic = topic.substr(12, topic.size() - 13);

							std::string regexStr = topic;
							regexStr = regex_replace(regexStr, regex("\\*"), ".*");
    						regexStr = std::regex_replace(regexStr, std::regex("\\+"), "[^/]+");
							removeIdTopic(received_packet.client, regexStr);
							continue;
						} else {
							// Invalid message - Ignore
							continue;
						}
					}
				}
			}
		}
  	}
}

int main(int argc, char *argv[]) {
  DIE(argc != 2, "Usage: ./server <port>");
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  // Parse the port as a number
  uint16_t port;
  int rc = sscanf(argv[1], "%hu", &port);
  DIE(rc != 1, "Given port is invalid");

  // Obtain a file descriptor for the TCP and UDP sockets
  const int tcpfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(tcpfd < 0, "socket");

  int flag = 1;
	setsockopt(tcpfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

  const int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
  DIE(udpfd < 0, "socket");

  // Complete the server address structure
  struct sockaddr_in serv_addr;
  socklen_t socket_len = sizeof(struct sockaddr_in);

  memset(&serv_addr, 0, socket_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  rc = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
  DIE(rc <= 0, "inet_pton");

  // bind the TCP and UDP sockets
  rc = bind(tcpfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(rc < 0, "bind");

  rc = bind(udpfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));

  
  run_chat_multi_server(tcpfd, udpfd);

  // Close the sockets
  close(tcpfd);

  return 0;
}
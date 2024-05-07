#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "helpers.hpp"
#include "common.hpp"


#define START_CLIENTS 2

#define MAX_CLIENT_ID_LENGTH 10
#define MAX_CLIENTS 128
#define MAX_UDP_MSG_SIZE 1552
#define MAX_UDP_PAYLOAD_SIZE 1500
#define MAX_TOPIC_NAME_LENGTH 51
#define MAX_TCP_PAYLOAD_SIZE 1601
#define SF_ENABLED 1
#define SF_DISABLED 0

/**
 * Structure representation for a udp message.
 */
struct udp_msg {
	char topic[MAX_TOPIC_NAME_LENGTH];
	unsigned char type;
	char data[MAX_UDP_PAYLOAD_SIZE];
};

struct subscribed_topic {
	std::string clientID;
	std::vector<std::string> topics;
};

// /**
//  * Structure representation for a tcp message.
//  */
// struct tcp_msg {
// 	char type;
// 	char data[MAX_TCP_PAYLOAD_SIZE];
// };

// /**
//  * Structure representation for a subscribe/unsubscribe message
//  * sent by the tcp clients.
//  */
// struct subscribe_msg {
// 	int type;
// 	char topic[MAX_TOPIC_NAME_LENGTH];
// 	int sf;
// };

struct TCPClient {
	std::string id;
	std::string ip;
	int port;
};


// // Define a struct to represent a UDP message
// struct UDPMessage {
//     std::string topic;
//     std::string payload;
// };

// int connNumber = 2;
std::map<std::string, std::vector<std::string>> topicsToIds;
std::map<std::string, std::vector<std::string>> idToTopics;


// Define a map to store the TCP clients
std::vector<TCPClient> tcpClients;


// Define a vector to store the UDP messages
std::vector<udp_msg> udpMessages;


// Function to handle TCP client connection
void handleTCPClientConnection(const std::string& clientId) {
	// Implement the logic to handle TCP client connection
}


// Function to handle TCP client disconnection
void handleTCPClientDisconnection(const std::string& clientId) {
	// Implement the logic to handle TCP client disconnection
}


// Function to handle UDP message reception
void handleUDPMessage(const std::string& topic, const std::string& payload) {
	// Implement the logic to handle UDP message reception
}

void removeIdTopic(std::string id, std::string topic) {
	for (long unsigned i = 0; i < idToTopics[id].size(); i++) {
		if (idToTopics[id][i] == topic) {
			idToTopics[id].erase(idToTopics[id].begin() + i);
			break;
		}
	}
	for (long unsigned i = 0; i < topicsToIds[topic].size(); i++) {
		if (topicsToIds[topic][i] == id) {
			topicsToIds[topic].erase(topicsToIds[topic].begin() + i);
			break;
		}
	}
}


void run_chat_multi_server(int listenfd) {

  pollfd *poll_fds = new pollfd[START_CLIENTS];
  int num_sockets = 2;
  int rc;

  chat_packet received_packet;

  // Setam socket-ul listenfd pentru ascultare
  rc = listen(listenfd, num_sockets);
  DIE(rc < 0, "listen");

  // Adaugam noul file descriptor (socketul pe care se asculta conexiuni) in
  // multimea poll_fds
  poll_fds[0].fd = listenfd;
  poll_fds[0].events = POLLIN;
  poll_fds[1].fd = STDIN_FILENO;
  poll_fds[1].events = POLLIN;

	while (1) {
		// Asteptam sa primim ceva pe unul dintre cei num_sockets socketi
		rc = poll(poll_fds, num_sockets, -1);
		DIE(rc < 0, "poll");

		for (int i = 0; i < num_sockets; i++) {
			if (poll_fds[i].revents & POLLIN) {
				if (poll_fds[i].fd == STDIN_FILENO) {
					char input[256];
					std::cin >> input;
					input[strlen(input)] = '\0'; // remove newline character

					if (strcmp(input, "exit") == 0) {
						// Close the router
						for (int i = 0; i < num_sockets; i++) {
						close(poll_fds[i].fd);
						}
						return;
					}
				} else if (poll_fds[i].fd == listenfd) {
					// Am primit o cerere de conexiune pe socketul de listen, pe care
					// o acceptam
					struct sockaddr_in cli_addr;
					socklen_t cli_len = sizeof(cli_addr);
					const int newsockfd =
						accept(listenfd, (struct sockaddr *)&cli_addr, &cli_len);
					DIE(newsockfd < 0, "accept");

					// Adaugam noul socket intors de accept() la multimea descriptorilor
					// de citire
					poll_fds = (pollfd*)realloc(poll_fds, (num_sockets + 1) * sizeof(pollfd));
					poll_fds[num_sockets].fd = newsockfd;
					poll_fds[num_sockets].events = POLLIN;
					num_sockets++;

					//   printf("Noua conexiune de la %s, port %d, socket client %d\n",
					//          inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port),
					//          newsockfd);

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
					} else {
						// Create a TCPClient object and add it to the map
						TCPClient newClient;
						newClient.id = std::string(clientID);
						newClient.ip = inet_ntoa(cli_addr.sin_addr);
						newClient.port = ntohs(cli_addr.sin_port);

						tcpClients.push_back(newClient);

						std::cout << "New client " << newClient.id << " connected from " << newClient.ip << ":" << newClient.port << std::endl;
					}
				} else {
					// Am primit date pe unul din socketii de client, asa ca le receptionam
					int rc = recv_all(poll_fds[i].fd, &received_packet,
										sizeof(received_packet));
					DIE(rc < 0, "recv");

					if (!strncmp(received_packet.message, "exit", 4)) {
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
							idToTopics[received_packet.client].push_back(topic);
							topicsToIds[topic].push_back(received_packet.client);

							
							// std::cout << "Subscribed to topic " << topic << std::endl;
							continue;
						} else if (!strncmp(received_packet.message, "unsubscribe", 11)) {
							std::string topic = received_packet.message;
							topic = topic.substr(12, topic.size() - 13);
							removeIdTopic(received_packet.client, topic);
							// std::cout << "Unsubscribed from topic " << topic << std::endl;
							continue;
						} else {
							// udp_msg udpMsg;
							// memcpy(&udpMsg, received_packet.message, sizeof(udpMsg));
							// std::cout << "Received message from topic " << udpMsg.topic << ": " << udpMsg.data << std::endl;
							// continue;
						}
						// printf("S-a primit de la clientul de pe socketul %d mesajul: %s\n",
						// 	poll_fds[i].fd, received_packet.message);
						/* TODO 2.1: Trimite mesajul catre toti ceilalti clienti */
						// for (int j = 1; j < num_sockets; j++) {
						// 	if (poll_fds[j].fd != poll_fds[i].fd) {
						// 		int rc = send_all(poll_fds[j].fd, &received_packet, sizeof(received_packet));
						// 		DIE(rc < 0, "send");
						// 	}
						// }
					}
				}
			}
		}
  	}
}

int main(int argc, char *argv[]) {
  DIE(argc != 2, "Usage: ./server <port>");
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  // Parsam port-ul ca un numar
  uint16_t port;
  int rc = sscanf(argv[1], "%hu", &port);
  DIE(rc != 1, "Given port is invalid");

  // Obtinem un socket TCP pentru receptionarea conexiunilor
  const int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(listenfd < 0, "socket");

  // Completăm in serv_addr adresa serverului, familia de adrese si portul
  // pentru conectare
  struct sockaddr_in serv_addr;
  socklen_t socket_len = sizeof(struct sockaddr_in);

  // Facem adresa socket-ului reutilizabila, ca sa nu primim eroare in caz ca
  // rulam de 2 ori rapid
  // Vezi https://stackoverflow.com/questions/3229860/what-is-the-meaning-of-so-reuseaddr-setsockopt-option-linux
  const int enable = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	perror("setsockopt(SO_REUSEADDR) failed");

  memset(&serv_addr, 0, socket_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  rc = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
  DIE(rc <= 0, "inet_pton");

  // Asociem adresa serverului cu socketul creat folosind bind
  rc = bind(listenfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(rc < 0, "bind");

  /*
	TODO 2.1: Folositi implementarea cu multiplexare
  */
//   run_chat_server(listenfd);
  run_chat_multi_server(listenfd);

  // Inchidem listenfd
  close(listenfd);

  return 0;
}
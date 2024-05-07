// #include <iostream>
// #include <vector>
// #include <iomanip>
// #include <unistd.h>
// #include <cstring>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <netinet/tcp.h>
// #include <arpa/inet.h>
// #include <cctype>

// // #include "./structures.hpp"
// #include "./helpers.hpp"
// #include "./common.hpp"

// using namespace std;

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

// #include "./structures.hpp"
#include "./helpers.hpp"
#include "./common.hpp"

using namespace std;

struct pollfd fds[2];
int pollfd_count = 2;


#define MAX_CLIENT_ID_LENGTH 10
#define MAX_CLIENTS 128
#define MAX_UDP_MSG_SIZE 1552
#define MAX_UDP_PAYLOAD_SIZE 1500
#define MAX_TOPIC_NAME_LENGTH 51
#define MAX_TCP_PAYLOAD_SIZE 1601
#define SF_ENABLED 1
#define SF_DISABLED 0



void run_client(int sockfd, char *client_id) {
  // cout << "Client ID: " << client_id << endl;
  char buf[MSG_MAXSIZE + 1];
  memset(buf, 0, MSG_MAXSIZE + 1);

  struct chat_packet sent_packet;
  struct chat_packet recv_packet;

  /*
    TODO 2.2: Multiplexati intre citirea de la tastatura si primirea unui
    mesaj, ca sa nu mai fie impusa ordinea.

    Hint: server::run_multi_chat_server
  */
  // struct pollfd fds[2];
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
      rc = recv_all(sockfd, &recv_packet, sizeof(recv_packet));
      if (rc <= 0) {
        break;
      }
      printf("%s\n", recv_packet.message);
    }

    if (fds[1].revents & POLLIN) {
      // cout << "Enter message: ";
      if (fgets(buf, sizeof(buf), stdin) && !isspace(buf[0])) {
        // cout << "Message: " << buf << endl;
        if (!strcmp(buf, "exit\n")) {
          // cout << "Subscribing to topic" << endl;
          chat_packet packet;
          strcpy(packet.client, client_id);
          packet.len = strlen(buf) + 1;
          strcpy(packet.message, buf);
          send_all(sockfd, &packet, sizeof(packet));
          break;
        }
        if (!strncmp(buf, "subscribe", 9)) {
          // cout << "Subscribing to topic" << endl;
          chat_packet packet;
          strcpy(packet.client, client_id);
          packet.len = strlen(buf) + 1;
          strcpy(packet.message, buf);
          send_all(sockfd, &packet, sizeof(packet));

          string topic = buf;
          topic = topic.substr(10, topic.size() - 11);
          cout << "Subscribed to topic " << topic << endl;
          continue;
        }
        if (!strncmp(buf, "unsubscribe", 11)) {
          // cout << "Unsubscribing from topic" << endl;
          chat_packet packet;
          strcpy(packet.client, client_id);
          packet.len = strlen(buf) + 1;
          strcpy(packet.message, buf);
          send_all(sockfd, &packet, sizeof(packet));
          string topic = buf;
          topic = topic.substr(12, topic.size() - 13);
          cout << "Unsubscribed from topic " << topic << endl;
          continue;
        }
        sent_packet.len = strlen(buf) + 1;
        strcpy(sent_packet.message, buf);

        // Trimitem pachetul la server.
        send_all(sockfd, &sent_packet, sizeof(sent_packet));
      }
    }
  }
}

int main(int argc, char *argv[]) {
  DIE(argc != 4, "Usage: ./subscriber <IP_SERVER> <PORT_SERVER>");
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  // Parsam port-ul ca un numar
  uint16_t port;
  int rc = sscanf(argv[3], "%hu", &port);
  DIE(rc != 1, "Given port is invalid");
  // Obtinem un socket TCP pentru conectarea la server
  const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(sockfd < 0, "socket");
  // Completăm in serv_addr adresa serverului, familia de adrese si portul
  // pentru conectare
  struct sockaddr_in serv_addr;
  socklen_t socket_len = sizeof(struct sockaddr_in);
  memset(&serv_addr, 0, socket_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
  DIE(rc <= 0, "inet_pton");
  // Ne conectăm la server
  rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(rc < 0, "connect");
    // Send the client ID to the server
    char* client_id = argv[1]; // Replace with your desired client ID

    send_all(sockfd, argv[1], MAX_CLIENT_ID_LENGTH);
  run_client(sockfd, argv[1]);
  // Inchidem conexiunea si socketul creat
  close(sockfd);
  return 0;
}

// int main(int argc, char *argv[]) {
//   DIE(argc != 4, "Usage: ./subscriber <IP_SERVER> <PORT_SERVER>");

//   // Parsam port-ul ca un numar
//   uint16_t port;
//   int rc = sscanf(argv[3], "%hu", &port);
//   DIE(rc != 1, "Given port is invalid");

//   // Obtinem un socket TCP pentru conectarea la server
//   const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//   DIE(sockfd < 0, "socket");

//   // Completăm in serv_addr adresa serverului, familia de adrese si portul
//   // pentru conectare
//   struct sockaddr_in serv_addr;
//   socklen_t socket_len = sizeof(struct sockaddr_in);

//   memset(&serv_addr, 0, socket_len);
//   serv_addr.sin_family = AF_INET;
//   serv_addr.sin_port = htons(port);
//   rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
//   DIE(rc <= 0, "inet_pton");

//   // Ne conectăm la server
//   rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
//   DIE(rc < 0, "connect");

//     // Send the client ID to the server
//     char* client_id = argv[1]; // Replace with your desired client ID
//     send_all(sockfd, client_id, sizeof(client_id));

//   run_client(sockfd, argv[1]);

//   // Inchidem conexiunea si socketul creat
//   close(sockfd);

//   return 0;
// }

# TCP and UDP Client-Server Application for Message Management

### Author: Nichita-Adrian Bunu, 323CA  
**Contact:** [nichita_adrian.bunu@stud.acs.upb.ro](mailto:nichita_adrian.bunu@stud.acs.upb.ro)

---

## Overview

This project implements a client-server application using both TCP and UDP protocols for message management. The system is composed of two main components:

- **`subscriber.cpp`**: Acts as a client, capable of sending and receiving TCP packets.
- **`server.cpp`**: Functions as a router, handling incoming TCP packets from clients and UDP packets from various topics.

---

## Features and Functionality

### `subscriber.cpp`

The implementation is based on the template provided in Laboratory 7. It simulates a client capable of:

- **Subscribing** to Topics:  
  Receives TCP packets from the router associated with a subscribed topic.

- **Unsubscribing** from Topics:  
  Stops receiving TCP packets from the router once a topic is unsubscribed.

#### Packet Handling

When a TCP packet containing a UDP message from the router is received, the client processes and displays the content according to the message type:  
- **INT**: Integer data type
- **SHORT_REAL**: Short real number
- **FLOAT**: Floating-point number
- **STRING**: Textual message

#### Error Handling

The program is designed to manage potential runtime errors gracefully, ensuring robust and reliable functionality (using the `DIE` function for error management).

---

## Future Improvements

The project can be extended with additional features, such as:
- **Enhanced Message Filtering:** Allowing clients to filter messages based on custom criteria.
- **Persistent Connections:** Implementing mechanisms for reconnection and message replay in case of disconnections.
- **Security Enhancements:** Adding encryption and authentication to ensure secure communication.

---

## How to Run

1. **Compile** the source code using a C++ compiler:
   ```bash
   g++ -o subscriber subscriber.cpp
   g++ -o server server.cpp
   ```

2. **Start the server**:
   ```bash
   ./server
   ```

3. **Run the client**:
   ```bash
   ./subscriber
   ```

---

Feel free to reach out for any questions or improvements!
# Chat C2S in C 
> by Sharp - 1st Year of Master's in Computer Security

## Table of contents
<details>
  <summary> Click me ⬇️ </summary>
  
  - [Project Context](#project-context)
  - [Description](#description)
      - [Program Features](#program-features)
      - [Error Management](#error-management)
      - [Specific Cases](#specific-cases)
  - [Program Structure & Architecture](#program-structure--architecture)
  - [Choices made](#choices-made)
  - [How to test?](#how-to-test)
  - [License](#license)
</details>

## Project Context

This project is part of a C course in my first year of Master's in Computer Security. The objective of the project is to develop a Chat Client-Server program in C, where multiple clients can communicate with each other through a server acting as an intermediary. The purpose of this project is to gain hands-on experience in C programming and network communication, as well as to understand the concepts of client-server architecture.

## Description

This program enables multiple clients to communicate with each other using a server as an intermediary. Clients connect to the server by registering their name and can send messages to other clients through the server.

### Program Features

- **Registration :** Clients can register with the server using a unique name.
- **Client Interaction :** Clients can request to speak by typing "1".
- **Exclusive Speaking :** Only one client can speak at a time, ensuring orderly communication.
- **Message Broadcasting :** The server displays the name of the sending client and their message to all other connected clients.
- **Mutex for Socket Access :** A mutex is used to protect access to write sockets, preventing competitive errors and ensuring message integrity.

### Error Management

The server handles various error cases to ensure smooth operation and graceful termination :

- **Client Limit :** The server checks if the number of clients has reached the limit before accepting new connections.
- **Bind Function Failure :** The server handles the case where the bind function fails and exits gracefully.
- **Listen Function Failure :** The server handles the case where the listen function fails and exits gracefully.
- **Send Function Failure :** The server handles the case where the send function fails and closes the client socket.

### Specific Cases

The program handles specific cases to enhance user experience and prevent issues :

- **Reconnection :** Clients have the possibility to connect or rejoin the server using the same port number.
- **Maximum Client Capacity :** The server can handle up to a maximum number of clients specified by MAX_CLIENTS.
- **Continuous Listening :** The server listens for incoming connections indefinitely until manually terminated.
- **Client Notification :** The server sends a message to all clients when a new client connects or disconnects.
- **Message Truncation :** If a client sends a message larger than the buffer size, it will be truncated to fit.
    - Client Connection Refusal : If the server reaches the maximum number of clients, it will refuse new connections and notify the client.

## Program Structure & Architecture

```bash
.
└─── chat_c2s_c
    ├─── src
           ├─── handle_client.c
           └─── server.c
    ├─── BUILD.md
    └─── README.md
```

The project is structured as follows:

- **handle_client.c :** Code for client-side functionality.
- **server.c :** Code for server-side functionality.

## Choices made

- **Modular Code :** The code is divided into small functions to facilitate code review and maintainability. There are a total of 9 functions, including the main function.

To ensure thread safety and prevent race conditions when accessing write sockets, a mutex is used. This allows only one client to speak at a time, ensuring messages are not lost or corrupted.

## How to test? 

To use this program, follow these steps:

1. Start the server first.
2. Open multiple terminals and connect to the server using the following command :
   `nc localhost <port>`  (e.g., `nc localhost 1234`).
3. Register with the server by providing a unique name.
4. Communicate with other clients by sending messages through the server.
5. Request to speak by typing "1" to indicate the intention to send a message. Only one client can speak at a time.

Please refer to the [BUILD.md](BUILD.md) file for instructions on building and running the program. Ensure dependencies mentioned in the file are installed.

Enjoy using this program and exploring client-server communication in C!

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for more information.

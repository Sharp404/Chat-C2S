#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define MAX_THREAD 10
#define NAME_SIZE 11

char client_names[MAX_CLIENTS][NAME_SIZE];
int clients[MAX_CLIENTS];
int num_clients = 0;
int server_socket;
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[MAX_THREAD];

// Signal handler function to handle cleanup
void cleanup(int signal_number) {

    (void)signal_number;
    const char msg[] = "Connection closed\n";
    const size_t msg_len = sizeof(msg);

    // Notify other clients that a client has left
    for (int i = 0; i < num_clients; i++) {
        pthread_mutex_lock(&socket_mutex);
        if (send(clients[i], msg, msg_len, 0) < 0) {
            perror("send failed");
            close(clients[i]); // close the socket
            for (int j = i; j < num_clients - 1; j++) {
                clients[j] = clients[j + 1]; // shift the remaining clients up in the list
            }
            i--; // stay on the same index, since the current client has been removed
            num_clients--;
        }
        pthread_mutex_unlock(&socket_mutex);
    }
    
    // Disconnecting clients and cleaning up threads
    pthread_mutex_destroy(&socket_mutex);
    num_clients = 0;
    close(server_socket);

    // Wait for all threads to finish before exiting
    for (int i = 0; i < MAX_THREAD; i++) {
        if (threads[i] != 0) {
            pthread_join(threads[i], NULL);
        }
    }
    _exit(EXIT_SUCCESS);
}

// Function to handle client communication
void* handle_client(void* arg);

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    
    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Enable reuse of port
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    // Initialize server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(1234);

    // Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "Error binding socket to port.\n");
        exit(EXIT_FAILURE);
    }

    // Listening for incoming client connections 
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        fprintf(stderr, "Error listening for incoming connections.\n");
        exit(EXIT_FAILURE);
    }
    printf("Server started.\n");

    // Registering signal handlers for cleanup
    signal(SIGINT, (void (*)(int)) cleanup);
    signal(SIGTERM, (void (*)(int)) cleanup);
    
    // Loop to accept client connections
    while (1) {

        // Accept incoming client connections
        client_socket = accept(server_socket, NULL, NULL);

        // Check if the number of clients has reached the limit
        if (num_clients >= MAX_CLIENTS) {
            fprintf(stderr, "Max clients reached. Connection refused.\n");
            close(client_socket);
            continue;
        }

        // Launching a thread to manage client communications
        pthread_create(&threads[num_clients], NULL, handle_client, &client_socket);

        // Adding the client to the client list
        pthread_mutex_lock(&socket_mutex);
        clients[num_clients] = client_socket;
        num_clients++;
        pthread_mutex_unlock(&socket_mutex);
    }
    cleanup(0);
}
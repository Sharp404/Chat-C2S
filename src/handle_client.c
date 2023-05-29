#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 31
#define MAX_CLIENTS 100
#define NAME_SIZE 11

extern char client_names[MAX_CLIENTS][NAME_SIZE];
extern int clients[MAX_CLIENTS];
extern int num_clients;
extern pthread_mutex_t socket_mutex;

void exit_clean(int client_socket);

void send_msg(int client_socket, char msg[]) {

    if (fcntl(client_socket, F_GETFD) == -1 || !msg)
        exit_clean(client_socket);
    pthread_mutex_lock(&socket_mutex);

    if (send(client_socket, msg, strlen(msg), MSG_NOSIGNAL) < 0) {
        fprintf(stderr, "Error sending message to client: %s\n", strerror(errno));
        exit_clean(client_socket);
    }
    pthread_mutex_unlock(&socket_mutex);
}

// Send a message to all connected clients 
void send_all(char msg[]) {
    for (int i = 0; i < num_clients; i++)
        send_msg(clients[i], msg);
}

// Removes a client from the list of connected clients and notifies all other clients of their disconnection
void disconnect_user(int client_socket) {
    int client_index = -1;
    char client_name[NAME_SIZE] = "";

    // Find the index and name of the client
    for (int i = 0; i < num_clients; i++) {
        if (clients[i] == client_socket) {
            client_index = i;
            strncpy(client_name, client_names[i], NAME_SIZE);
            break;
        }
    }
    
    // Remove the client from the list of clients
    if (client_index != -1) {
        for (int j = client_index; j < num_clients - 1; j++) {
            clients[j] = clients[j+1];
            strncpy(client_names[j], client_names[j+1], NAME_SIZE);
        }
        num_clients--;
    }

    // Notify all other clients of the disconnection
    if (client_index != -1) {
        char disconnect_message[strlen(client_name) + strlen(" has left the chat.\n")];
        if (client_name[0] == '\0') {
            sprintf(disconnect_message, "Unknown has left the chat.\n");
        } else {
            sprintf(disconnect_message, "%s has left the chat.\n", client_name);
        }
        send_all(disconnect_message);

        // Update the names of the remaining clients in client_names
        for (int i = client_index; i < num_clients; i++) {
            strncpy(client_names[i], client_names[i+1], NAME_SIZE);
        }
    }
    close(client_socket);
    pthread_exit(NULL);
}

void exit_clean(int client_socket) {
    disconnect_user(client_socket);
    close(client_socket);
    pthread_exit(NULL);
}

void recv_msg(int client_socket, char buffer[], int size) {
    int recv_size = recv(client_socket, buffer, size - 1, 0);
    if (recv_size <= 0) {
        if (recv_size == 0)
            fprintf(stderr, "Client disconnected.\n");
        else if (recv_size < 0)
            fprintf(stderr, "Error receiving message from client: %s\n", strerror(errno));
        exit_clean(client_socket);
    }
    buffer[recv_size] = 0;
    buffer[strcspn(buffer, "\n")] = 0;
    buffer[strcspn(buffer, "\r")] = 0;
}

// Set the name of a client and registers it with the server
void set_name(int client_socket, char client_name[NAME_SIZE]) {
    bool name_valid = false;
    do {
        send_msg(client_socket, "Enter your name (10 characters max) : ");

        // Receive client's Name
        recv_msg(client_socket, client_name, NAME_SIZE);

        // Check if the name is empty or filled with spaces
        if (strlen(client_name) == 0 || strspn(client_name, " ") == strlen(client_name)) {
            send_msg(client_socket, "Name can't be empty or only contain spaces, \nPlease enter a valid name: ");
            continue;
        }

        // Check if the name is already in use
        bool name_used = false;
        for (int i = 0; i < num_clients; i++) {
            if (strcmp(client_names[i], client_name) == 0) {
                name_used = true;
                break;
            }
        }

        if (name_used) {
            send_msg(client_socket, "This name is already taken, \nPlease choose another name : ");
            continue;
        }
        name_valid = true;

    } while (!name_valid);

    // Registering the client’s name with the server
    strncpy(client_names[num_clients-1], client_name, NAME_SIZE); // num_clients-1 because we have already increased the number of clients in the main()
}

// Function to handle client communication
void *handle_client(void* arg) {

    int client_socket = *((int*)arg);
    char buffer[BUFFER_SIZE];
    char client_name[NAME_SIZE] = "";

    set_name(client_socket, client_name);
    
    // Displaying Client Login Message
    int message_length = '\n' + strlen(client_name) + strlen(" is connected\n") + 1;
    char message[message_length];
    sprintf(message, "\n%s is connected\n", client_name);
    send_all(message);

    // Loop to send and receive messages
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // Request for permission to speak
        send_msg(client_socket, "Press 1 to speak :\n");

        // Receive permission to speak
        recv_msg(client_socket, buffer, BUFFER_SIZE);        

        if (strcmp(buffer, "1") == 0) {

            // Request message
            send_msg(client_socket, "Enter your message (30 characters max) :\n");

            // Receive message
            bool message_received = false;
            while (!message_received) {
                memset(buffer, 0, BUFFER_SIZE);
                recv_msg(client_socket, buffer, BUFFER_SIZE);

                // Check if the message is empty or filled with spaces
                if (strlen(buffer) == 0 || strspn(buffer, " ") == strlen(buffer)) {
                    send_msg(client_socket, "Message can't be empty or only contain spaces, \nPlease enter a valid message: ");
                }

                // Check if the message is too long
                else if (strlen(buffer) > 30) {
                    send_msg(client_socket, "Message is too long (max 30 characters), \nPlease enter a shorter message: ");
                }
                else {
                    message_received = true;
                }
            }

            // Format the message to include the client's name and their message
            int message_length = strlen(client_name) + strlen(buffer) + 6; // +5 for "[] : " characters
            char message[message_length];
            int offset = sprintf(message, "[%s] : ", client_name);
            strncpy(message + offset, buffer, message_length - offset);
            message[message_length] = 0;
            
            // Send message to all customers
            send_all(message);
        
            // Add a newline after sending the message
            send_msg(client_socket, "\n");
        }
    }
    exit_clean(client_socket);
}

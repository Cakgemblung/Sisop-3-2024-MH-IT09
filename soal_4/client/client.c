#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_ADDRESS "127.0.0.1"
#define MAX_BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE] = {0};

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    // Send message to server
    printf("Enter command: ");
    fgets(buffer, MAX_BUFFER_SIZE, stdin);
    send(client_socket, buffer, strlen(buffer), 0);
    printf("Message sent to server: %s\n", buffer);

    // Receive response from server
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    printf("Response from server: %s\n", buffer);

    // Close socket
    close(client_socket);

    return 0;
}

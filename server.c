// server code
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

//int main(int argc, char const* argv[]) {
int main() {
    // Set up server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(server_fd, (const struct sockaddr *) &addr, sizeof(addr));

    // Listen for connections
    listen(server_fd, 5);

    // Accept client connection
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &client_addrlen);

    // Print client ip and port
    char *ip_str = inet_ntoa(client_addr.sin_addr);
    printf("Connection from %s:%d\n", ip_str, ntohs(client_addr.sin_port));

    // Send client message
    char name_request[] = "Hello client! What is your name?\n";
    send(client_fd, name_request, strlen(name_request), 0);


    // Listen for client response
    char name_buffer[10];
    int bytes_read = recv(client_fd, name_buffer, sizeof(name_buffer) - 1, 0);

    if (bytes_read > 0) {
        name_buffer[bytes_read] = '\0';
        printf("Client says: %s\n", name_buffer);
    }
    
    // Say hello!
    char hello_message[20];
    snprintf(hello_message, sizeof(hello_message), "Hello %s!\n", name_buffer);
    send(client_fd, hello_message, strlen(hello_message), 0);
}


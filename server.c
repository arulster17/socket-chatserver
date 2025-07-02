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

    char received_msg[] = "Received Message!\n";

    // Listen for client messages
    char buffer[1024];
    int n;
    while ((n = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("Client: %s", buffer);

        // Send acknowledgment (testing)
        send(client_fd, received_msg, strlen(received_msg), 0);
        
    }
    printf("Client disconnected");
    close(client_fd);
}


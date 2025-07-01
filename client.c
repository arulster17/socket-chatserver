// client code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

//int main(int argc, char const* argv[]) {
int main() {
    // Set up client socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        printf("client_fd is -1\n");
        exit(1);
    }

    // Connect to server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); // use same port as server
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Send a message

    char msg[] = "Hello from client!";
    send(client_fd, msg, strlen(msg), 0);


}

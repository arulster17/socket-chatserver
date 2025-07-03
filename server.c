// server code
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 10

int client_list[MAX_CLIENTS];
int available_client[MAX_CLIENTS];
pthread_mutex_t client_list_lock = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    // This will be called whenever a new client connects
    // arg will have the client fd
    int client_fd = *(int *) arg;
    int n;
    char buffer[1024];

    // listen for client messages constantly
    while((n = recv(client_fd, buffer, sizeof(buffer)-1, 0)) > 0) {
        // Received message
        buffer[n] = '\0';
        printf("Client: %s", buffer);
        // Reflect message
        send(client_fd, buffer, strlen(buffer), 0);
    }
    // Connection closed
    printf("Client disconnected");
    return NULL;

}

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

    while(1) {
        int client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &client_addrlen);
        pthread_t handle_client_thread;
        pthread_create(&handle_client_thread, NULL, handle_client, &client_fd);
    }

/*





    // Print client ip and port
    char *ip_str = inet_ntoa(client_addr.sin_addr);
    printf("Connection from %s:%d\n", ip_str, ntohs(client_addr.sin_port));
    printf("server_fd: %d\n", server_fd);
    printf("client_fd: %d\n", client_fd);
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
    */
}



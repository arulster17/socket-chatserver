// server code
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <dispatch/dispatch.h>

#define PORT 8080
#define MAX_CLIENTS 2

int server_fd;
int client_list[MAX_CLIENTS];
int available_client[MAX_CLIENTS];
dispatch_semaphore_t client_slots;

struct threadinfo {
    int fd;
    int id;
};

void sigint_handler(int arg) {
    printf("\nShutting down the server\n");
    close(server_fd);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if(!available_client[i]) {
            // Goodbye clients
            close(client_list[i]);
        }
    }
    exit(0);
}

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
    printf("Client disconnected\n");
    dispatch_semaphore_signal(client_slots);
    close(client_fd);
    return NULL;

}

//int main(int argc, char const* argv[]) {
int main() {

    signal(SIGINT, sigint_handler);

    printf("hello\n");
    // Set up server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(server_fd, (const struct sockaddr *) &addr, sizeof(addr));

    // Listen for connections
    listen(server_fd, 5);
   

    client_slots = dispatch_semaphore_create(MAX_CLIENTS);

    // Accept client connection
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int cnt = 1;
    while(1) {
        // Wait for an open slot
        printf("%d waiting for open slot\n", cnt);
        dispatch_semaphore_wait(client_slots, DISPATCH_TIME_FOREVER);
        printf("%d got open slot\n", cnt);

        int client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &client_addrlen);
        pthread_t handle_client_thread;

        int *fd_ptr = malloc(sizeof(int));
        *fd_ptr = client_fd;
        pthread_create(&handle_client_thread, NULL, handle_client, fd_ptr);

        printf("%d connected to server\n", cnt);
        cnt++;
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



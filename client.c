// client code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_NAME_LEN 20

void print_peer_info(int sockfd) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getpeername(sockfd, (struct sockaddr*)&addr, &addr_len) == -1) {
        perror("getpeername failed");
        return;
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip_str, sizeof(ip_str));
    int port = ntohs(addr.sin_port);

    printf("Socket %d connected to %s:%d\n", sockfd, ip_str, port);
}

void *send_loop(void *arg) {
    int socket = *(int*) arg;
    char msg[1024];
    while (fgets(msg, sizeof(msg), stdin)) {
        printf("\033[F\033[K");

        send(socket, msg, strlen(msg), 0);
    }
    return NULL;
}

void *receive_loop(void *arg) {
    int sock = *(int*) arg;
    char buffer[1024];
    int n;
    while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    printf("Server disconnected.\n");
    close(sock);
    exit(0);
}


//int main(int argc, char const* argv[]) {
int main() {
    // Set up client socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("client_fd is -1\n");
        exit(1);
    }
    char username[MAX_NAME_LEN];
    printf("Enter username (max 16 chars): ");
    fgets(username, MAX_NAME_LEN, stdin);
    int user_len = strlen(username);
    while(username[user_len-1] == '\r' || username[user_len-1] == '\n') {
        username[--user_len] = '\0';
    } 
    // Connect to server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); // use same port as server
    //inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    //inet_pton(AF_INET, "24.6.51.191", &server_addr.sin_addr);
    inet_pton(AF_INET, "192.168.86.133", &server_addr.sin_addr);

    connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    print_peer_info(socket_fd);

    // poke to force handshake
    char poke = '\0';
    send(socket_fd, &poke, 1, 0);

    // set up username
    
    send(socket_fd, username, user_len, 0);

    // Set up sending and receiving threads
    pthread_t send_thread, recv_thread;
    pthread_create(&send_thread, NULL, send_loop, &socket_fd);
    pthread_create(&recv_thread, NULL, receive_loop, &socket_fd);

    
    // Wait for threads to finish i guess
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    // Listen for server message
    char buffer[1024];
    int bytes_read = recv(socket_fd, buffer, sizeof(buffer)-1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Server says: %s", buffer);
    }

    // Respond to server
    char name[100];
    fgets(name, sizeof(name), stdin);

    send(socket_fd, name, strlen(name), 0);

    close(socket_fd);
}





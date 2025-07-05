// client code
#include "commons.h"

void *send_loop(void *arg) {
    int socket = *(int*) arg;
    
    char *msg = NULL;
    int chars_to_read;

    while ((chars_to_read = read_n_string(&msg, -1)) > 0) {
        printf("\033[F\033[K");
        msg[chars_to_read] = '\n';
        send(socket, msg, chars_to_read+1, 0);
        free(msg);
        msg = NULL;
    }

    // EOF or some weird error
    close(socket);
    printf("send_loop ended\n");
    return NULL;
}

void *receive_loop(void *arg) {
    int socket = *(int*) arg;
    char buffer[1024];
    int n;
    while ((n = recv(socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    printf("Server disconnected.\n");
    close(socket);
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
    // send length
    printf("Enter username (max 16 chars): ");
    fflush(stdout);
    char *line = NULL;
    int chars_to_read = read_n_string(&line, MAX_NAME_LEN);
    if (chars_to_read == -1) {
        exit(1);
    }
    send(socket_fd, &chars_to_read, 1, 0);
    send(socket_fd, line, chars_to_read, 0);
    free(line);

    // Set up sending and receiving threads
    pthread_t send_thread, recv_thread;
    pthread_create(&send_thread, NULL, send_loop, &socket_fd);
    pthread_create(&recv_thread, NULL, receive_loop, &socket_fd);

    
    // Wait for threads to finish i guess
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
}





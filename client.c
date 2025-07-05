// client code
#include "commons.h"

void read_string(char **line_ptr, int *len, int maxlen) {
    size_t slen = 0;
    ssize_t read = getline(line_ptr, &slen, stdin);
    char *line = *line_ptr;
    // read = num letters + newline/eof
    
    if (read == -1) {
        printf("read == -1\n");
        exit(1);
    }
    if (read > 0 && line[read - 1] == '\n') {
            line[--read] = '\0';
    }
    int chars_to_read = maxlen;
    if (read < chars_to_read) {
        chars_to_read = read;
    }
    *len = chars_to_read;
    // line is now the chars + \0
}

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
    printf("Enter username (max 16 chars): ");
    fflush(stdout);

    char *line = NULL;
    int chars_to_read;
    read_string(&line, &chars_to_read, MAX_NAME_LEN);
    printf("line = %s\n", line);
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





// client code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_NAME_LEN 16

void read_string(char *s, int width)
{
    if (fgets(s, width, stdin) != 0)
    {
        size_t length = strlen(s);
        if (length > 0 && s[length-1] != '\n')
        {
            printf("flushing\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
        }
        return;
    }
    else {
        printf("idfk\n");
        exit(1);
    }

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
    char username[MAX_NAME_LEN+1];
    printf("Enter username (max 16 chars): ");
    fflush(stdout);
    char *line = NULL;
    size_t len = 0;
    ssize_t read = getline(&line, &len, stdin);
    // read = num letters + newline/eof
    
    if (read == -1) {
        printf("read == -1\n");
        exit(1);
    }
    if (read > 0 && line[read - 1] == '\n') {
            line[--read] = '\0';
    }
    int chars_to_read = MAX_NAME_LEN;
    if (read < chars_to_read) {
        chars_to_read = read;
    }
    // line is now the chars + \0

    printf("read is %d\n", chars_to_read);


    /*fgets(username, MAX_NAME_LEN+1, stdin);
    int user_len = strlen(username);
    
    if (!strchr(username, '\n')) {
        flush_stdin();
    }
    while(username[user_len-1] == '\r' || username[user_len-1] == '\n') {
        username[--user_len] = '\n';
    }
    username[user_len] = '\0';
    printf("user len is %d\n", user_len);
    read_string(username, MAX_NAME_LEN+1);
    int user_len = strlen(username);
    
    printf("username is %s\n", username);
    printf("user_len is %d\n", user_len);
    */
    int user_len = 16;
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
    
    printf("user_len is %d\n", user_len);
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





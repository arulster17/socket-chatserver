// client code
#include "commons.h"

// This thread handles clients sending messages 
void *send_loop(void *arg) {
    int socket = *(int*) arg;

    char *msg = NULL;
    int chars_to_read;

    while ((chars_to_read = read_n_string(&msg, -1)) >= 0) {
        // Read in input, basic filter, send the input
        printf("\033[F\033[K");
        msg[chars_to_read] = '\n';
        for (int i = 0; i < chars_to_read; i++) {
            // strip ascii 0-31 because they are bad news
            if (msg[i] < 32) {
                msg[i] = 32; 
            }
        }
        send(socket, msg, chars_to_read+1, 0);
        free(msg);
        msg = NULL;
    }

    // EOF or some weird error
    close(socket);
    printf("send_loop ended\n");
    return NULL;
}

// This thread handles clients receiving messages.
void *receive_loop(void *arg) {
    // Recv input, and just print to stdout
    int socket = *(int*) arg;
    char buffer[1024];
    int n;
    while ((n = recv(socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    
    // If the server disconnects this runs
    printf("Server disconnected.\n");
    close(socket);
    exit(0);
}


int main(int argc, char const* argv[]) {
    // Input handling stuff
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP> <port>\n", argv[0]);
        return 1;
    }

    const char *ip_str = argv[1];
    const char *port_str = argv[2];

    int port = atoi(port_str);
    if (port <= 0 || port > 65535) {
        printf("Invalid port number: %s\n", port_str);
        exit(1);
    }

    // Set up client socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("client_fd is -1\n");
        exit(1);
    }
    
    // Connect to server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); // use same port as server
    inet_pton(AF_INET, ip_str, &server_addr.sin_addr); 

    connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    print_peer_info(socket_fd);

    // poke to force handshake
    char poke = '\0';
    send(socket_fd, &poke, 1, 0);

    // set up username
    // send length first
    printf("Enter username (max 16 chars): ");
    fflush(stdout);
    char *line = NULL;
    int chars_to_read = read_n_string(&line, MAX_NAME_LEN);
    if (chars_to_read == -1) {
        // weird stuff happened while reading username
        close(socket_fd);
        exit(1);
    }
    line[chars_to_read] = '\n';
    chars_to_read++;
    printf("banana\n");
    send(socket_fd, &chars_to_read, 1, 0);
    send(socket_fd, line, chars_to_read, 0);
    free(line);

    // Set up sending and receiving threads
    pthread_t send_thread, recv_thread;
    pthread_create(&send_thread, NULL, send_loop, &socket_fd);
    pthread_create(&recv_thread, NULL, receive_loop, &socket_fd);

    // Hate to be that guy but i'm not really sure why we need these
    // But it breaks without them LOL
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
}





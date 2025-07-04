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
#define MAX_CLIENTS 9

struct client_information {
    int fd;
    int free;
    int name;
};

struct threadargs {
    int fd;
    int id;
};

int server_fd;
struct client_information client_list[MAX_CLIENTS];
dispatch_semaphore_t client_slots;
pthread_mutex_t shared_mem_lock;


void print_peer_info(int fd) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getpeername(fd, (struct sockaddr *)&addr, &addr_len) == -1) {
        perror("getpeername failed");
        return;
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip_str, sizeof(ip_str));
    int port = ntohs(addr.sin_port);

    printf("Socket fd %d connected to %s:%d\n", fd, ip_str, port);
}


void sigint_handler(int arg) {
    printf("\nShutting down the server\n");
    close(server_fd);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if(!client_list[i].free) {
            // Goodbye clients
            close(client_list[i].fd);
        }
    }
    pthread_mutex_destroy(&shared_mem_lock);
    exit(0);
}

void *handle_client(void *arg) {
    // This will be called whenever a new client connects
    // arg will have the client fd
    struct threadargs *t_args = (struct threadargs *) arg;
    int client_fd = t_args->fd;
    int thread_id = t_args->id;
    free(t_args);
    int n;
    char buffer[1000];
    char buffer2[2000];
    char poke;
    
    // handle poke
    n = recv(client_fd, &poke, 1, 0);


    // listen for client messages constantly
    while((n = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        // Received message
        buffer[n] = '\0';
        printf("Client %d: %s", client_list[thread_id].name, buffer);
        // Reflect message
        //send(client_fd, buffer, strlen(buffer), 0);

        // send message to other clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if(!client_list[i].free) {
                snprintf(buffer2, sizeof(buffer2), "Client %d: %s", 
                        client_list[thread_id].name, buffer);

                printf("Sending to client %d: %s", client_list[i].name, buffer2);
                int sendsuc = send(client_list[i].fd, buffer2, strlen(buffer2), 0);
                if (sendsuc == -1) {
                    printf("send failed\n");
                }
                else {
                    printf("send succeeded\n");
                }
                print_peer_info(client_list[i].fd);

            }
        }

    }
    // Connection closed
    printf("Client disconnected\n");
   
    // Mark slot available
    pthread_mutex_lock(&shared_mem_lock);
    client_list[thread_id].free = 1;
    pthread_mutex_unlock(&shared_mem_lock);

    // Signal semaphore and close client socket
    dispatch_semaphore_signal(client_slots);
    close(client_fd);
    return NULL;

}

//int main(int argc, char const* argv[]) {
int main() {
    
    // Initialize synchronization variables
    pthread_mutex_init(&shared_mem_lock, NULL);
    client_slots = dispatch_semaphore_create(MAX_CLIENTS);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        printf("%d\n", i);
        client_list[i].free = 1;
    }

    // Set up SIGINT handler
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


    // Accept client connection
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int cnt = 1;
    while(1) {
        // Wait for an open slot
        printf("%d waiting for open slot\n", cnt);
        dispatch_semaphore_wait(client_slots, DISPATCH_TIME_FOREVER);
        printf("%d got open slot\n", cnt);

        // Build thread and arguments
        pthread_t handle_client_thread;
        int client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &client_addrlen);
        printf("accepted \n");
                // Given we are here, there must be an open client slot, find first
        int openslot = 0;
        
        pthread_mutex_lock(&shared_mem_lock);
        while (openslot < MAX_CLIENTS) {
            printf("checking slot %d\n", openslot);
            printf("value is %d\n", client_list[openslot].free);
            if(client_list[openslot].free) {
                break;
            }
            openslot++;
        }
        if (openslot == MAX_CLIENTS) {
            // Something wacky has happened
            printf("openslot == MAX_CLIENTS\n");
            while(1) {}
            // this is called good code
        }
        // Register new thread
        printf("placed at slot %d\n", openslot);
        client_list[openslot].fd = client_fd;
        client_list[openslot].free = 0;
        client_list[openslot].name = cnt;
        pthread_mutex_unlock(&shared_mem_lock);
        
        struct threadargs *ta_ptr = malloc(sizeof(struct threadargs));
        ta_ptr->fd=client_fd;
        ta_ptr->id=openslot;
        //ta_ptr->name=cnt;

        // if this deadlocks i will cry

        pthread_create(&handle_client_thread, NULL, handle_client, ta_ptr);

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



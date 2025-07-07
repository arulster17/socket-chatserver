// server code
#include "commons.h"

// struct containing information for a given client
struct client_information {
    int fd;
    int free;
    char name[MAX_NAME_LEN+1];
    int color;
};

// arguments passed to start a handle_client thread
struct threadargs {
    int fd;
    int id;
};

// shared memory and synchronization stuff
int server_fd;
struct client_information client_list[MAX_CLIENTS];
dispatch_semaphore_t client_slots;
pthread_mutex_t shared_mem_lock;

// handle server sigint cleanly
void sigint_handler(int arg) {
    printf("\nShutting down the server...\n");
    close(server_fd);
    pthread_mutex_lock(&shared_mem_lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if(!client_list[i].free) {
            // Goodbye clients
            close(client_list[i].fd);
        }
    }
    pthread_mutex_unlock(&shared_mem_lock);
    pthread_mutex_destroy(&shared_mem_lock);
    printf("Shutdown complete!\n");
    exit(arg);
}

// This will be called whenever a new client connects
void *handle_client(void *arg) {
    struct threadargs *t_args = (struct threadargs *) arg;
    int client_fd = t_args->fd;
    int thread_id = t_args->id;
    free(t_args);
    int n;
    char buffer[1000];
    char buffer2[2000];
   
    // get username
    int len;
    n = recv(client_fd, &len, 1, 0);
    char username[MAX_NAME_LEN+1];
    n = recv(client_fd, &username, len, 0);
    username[len-1] = '\0';
 

    
    // clear screen and make welcome bar
    char clrscrn[] = "\033[2J\033[H";
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    char *weq = (char *) malloc(w.ws_col+1);
    memset(weq, '=', w.ws_col);
    weq[w.ws_col] = '\n';
    char welcome[] = "Welcome to the chatroom!\nCurrent users:";
    
    pthread_mutex_lock(&shared_mem_lock);

    // write username to the shared memory
    strncpy(client_list[thread_id].name, username, n);
    client_list[thread_id].name[n] = '\0';

    // print out the clearscreen and welcome bar
    send(client_fd, clrscrn, strlen(clrscrn), 0);
    send(client_fd, weq, w.ws_col+1, 0);
    send(client_fd, welcome, strlen(welcome), 0);
    

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!client_list[i].free) {
            // Print users that are already in the room
            snprintf(buffer2, sizeof(buffer2), " \033[1;%dm%s\033[0m", 
                        client_list[i].color, client_list[i].name);
            send(client_fd, buffer2, strlen(buffer2), 0);

            // Let others know that you joined
            if (i != thread_id) {
                snprintf(buffer2, sizeof(buffer2), ">>> \033[1;%dm%s\033[0m joined the room\n", 
                        client_list[thread_id].color, client_list[thread_id].name);
                send(client_list[i].fd, buffer2, strlen(buffer2), 0);

            }
        }
    }

    // Write in the server log that you joined
    printf(">>> \033[1;%dm%s\033[0m joined the room\n", 
                        client_list[thread_id].color, client_list[thread_id].name);

    int nl = '\n';
    send(client_fd, &nl, 1, 0);

    pthread_mutex_unlock(&shared_mem_lock);
    free(weq);

    // listen for client messages constantly
    while((n = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        // Received message
        buffer[n] = '\0';
        
        pthread_mutex_lock(&shared_mem_lock);
        
        // log message in the server window
        printf("\033[1;%dm%s\033[0m: %s", 
                client_list[thread_id].color, client_list[thread_id].name, buffer);

        // send message to all clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if(!client_list[i].free) {
                snprintf(buffer2, sizeof(buffer2), "\033[1;%dm%s\033[0m: %s", 
                        client_list[thread_id].color, client_list[thread_id].name, buffer);

                send(client_list[i].fd, buffer2, strlen(buffer2), 0);

            }
        }
        pthread_mutex_unlock(&shared_mem_lock);

    }
    
    // Connection closed by client
    // Mark slot available and notify others
    pthread_mutex_lock(&shared_mem_lock);
    client_list[thread_id].free = 1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!client_list[i].free) {
            // notify othere clients
            snprintf(buffer2, sizeof(buffer2), "<<< \033[1;%dm%s\033[0m left the room\n", 
                    client_list[thread_id].color, client_list[thread_id].name);
            send(client_list[i].fd, buffer2, strlen(buffer2), 0);
        }
    }

    // log disconnection to server
    printf("<<< \033[1;%dm%s\033[0m left the room\n", 
                    client_list[thread_id].color, client_list[thread_id].name);

    pthread_mutex_unlock(&shared_mem_lock);
    
    
    // Signal semaphore and close client socket
    dispatch_semaphore_signal(client_slots);
    close(client_fd);
    return NULL;

}

int main(int argc, char const* argv[]) {
    // Input handling stuff
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    const char *port_str = argv[1];

    int port = atoi(port_str);
    if (port <= 0 || port > 65535) {
        printf("Invalid port number: %s\n", port_str);
        exit(1);
    }
    
    // Seed random
    srand(time(NULL));

    // Initialize synchronization variables
    pthread_mutex_init(&shared_mem_lock, NULL);
    client_slots = dispatch_semaphore_create(MAX_CLIENTS);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_list[i].free = 1;
    }

    // Set up SIGINT handler
    signal(SIGINT, sigint_handler);
    
    // Set up server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(server_fd, (const struct sockaddr *) &addr, sizeof(addr));

    // Listen for connections
    listen(server_fd, 5);


    // Accept client connection loop
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    while(1) {
        // Wait for an open slot
        dispatch_semaphore_wait(client_slots, DISPATCH_TIME_FOREVER);

        // Build thread and arguments
        pthread_t handle_client_thread;
        int client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &client_addrlen);
        
        // poke the client to ensure handshake finishes
        char poke;
        int n = recv(client_fd, &poke, 1, 0);
        if (n == -1) {
            printf("poke failed");
            dispatch_semaphore_signal(client_slots);
            close(client_fd);
        }
                
        // Given we are here, there must be an open client slot, find first
        int openslot = 0;
        pthread_mutex_lock(&shared_mem_lock);
        while (openslot < MAX_CLIENTS) {
            if(client_list[openslot].free) {
                break;
            }
            openslot++;
        }
        if (openslot == MAX_CLIENTS) {
            // Something wacky has happened
            printf("openslot == MAX_CLIENTS\n");
            raise(SIGINT);
        }
        
        

        // Register new thread
        client_list[openslot].fd = client_fd;
        client_list[openslot].free = 0;
        client_list[openslot].color = 31+(rand() % 6);

        pthread_mutex_unlock(&shared_mem_lock);
        
        struct threadargs *ta_ptr = malloc(sizeof(struct threadargs));
        ta_ptr->fd=client_fd;
        ta_ptr->id=openslot;

        // if this deadlocks i will cry

        pthread_create(&handle_client_thread, NULL, handle_client, ta_ptr);

    }
}



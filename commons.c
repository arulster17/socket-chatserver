#include "commons.h"

int read_n_string(char **line_ptr, int maxlen) {
    size_t slen = 0;
    ssize_t read = getline(line_ptr, &slen, stdin);
    char *line = *line_ptr;
    // read = num letters + newline/eof
    
    if (read == -1) {
        // some client side error
        printf("read == -1\n");
        // mark len as -1
        return -1;
    }
    if (read > 0 && line[read - 1] == '\n') {
            line[--read] = '\0';
    }
    if (maxlen == -1) {
        // allow any length
        return read;
    }
    int chars_to_read = maxlen;
    if (read < chars_to_read) {
        chars_to_read = read;
    }
    line[chars_to_read] = '\0';
    return chars_to_read;
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

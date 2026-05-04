#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 9000
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define MAX_TOPIC 50
#define MAX_SUBS 20 

typedef struct {
    int fd;
    int sub_count; 
    char topics[MAX_SUBS][MAX_TOPIC];
} Client;

Client clients[MAX_CLIENTS];

void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].sub_count = 0;
    }
}

int find_client(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd == fd)
            return i;
    }
    return -1;
}

int add_client(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd == -1) {
            clients[i].fd = fd;
            clients[i].sub_count = 0;
            return i;
        }
    }
    return -1;
}

void remove_client(int index) {
    close(clients[index].fd);
    clients[index].fd = -1;
    clients[index].sub_count = 0;
}

void broadcast(char *topic, char *msg) {
    char out[BUFFER_SIZE];
    sprintf(out, "[%s] %s\n", topic, msg);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd != -1) {
            for (int j = 0; j < clients[i].sub_count; j++) {
                if (strcmp(clients[i].topics[j], topic) == 0) {
                    send(clients[i].fd, out, strlen(out), 0);
                    break; 
                }
            }
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    struct pollfd fds[MAX_CLIENTS + 1];
    char buffer[BUFFER_SIZE];

    init_clients();
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    printf("Pub/Sub Server chạy tại port %d...\n", PORT);
    for (int i = 0; i < MAX_CLIENTS + 1; i++)
        fds[i].fd = -1;

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    int nfds = 1;

    while (1) {
        poll(fds, nfds, -1);
        if (fds[0].revents & POLLIN) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

            int idx = add_client(new_socket);
            if (idx != -1) {
                fds[nfds].fd = new_socket;
                fds[nfds].events = POLLIN;
                nfds++;

                printf("Client connected (fd=%d)\n", new_socket);
            }
        }
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                int sd = fds[i].fd;

                int len = recv(sd, buffer, BUFFER_SIZE - 1, 0);

                if (len <= 0) {
                    int idx = find_client(sd);
                    if (idx != -1) remove_client(idx);

                    close(sd);
                    printf("Client disconnected (fd=%d)\n", sd);

                    for (int j = i; j < nfds - 1; j++)
                        fds[j] = fds[j + 1];

                    nfds--;
                    i--;
                    continue;
                }

                buffer[len] = '\0';
                buffer[strcspn(buffer, "\r\n")] = 0; 

                printf("Recv from fd=%d: %s\n", sd, buffer);

                int idx = find_client(sd);
                if (idx == -1) continue;
                if (strncmp(buffer, "SUB ", 4) == 0) {
                    char topic[MAX_TOPIC];
                    sscanf(buffer + 4, "%s", topic);
                    int is_subbed = 0;
                    for (int j = 0; j < clients[idx].sub_count; j++) {
                        if (strcmp(clients[idx].topics[j], topic) == 0) {
                            is_subbed = 1;
                            break;
                        }
                    }

                    if (is_subbed) {
                        send(sd, "Already subscribed\n", 19, 0);
                    } else if (clients[idx].sub_count < MAX_SUBS) {
                        strcpy(clients[idx].topics[clients[idx].sub_count], topic);
                        clients[idx].sub_count++;
                        send(sd, "Subscribed OK\n", 14, 0);
                    } else {
                        send(sd, "Max topics reached\n", 19, 0);
                    }
                }
                else if (strncmp(buffer, "UNSUB ", 6) == 0) {
                    char topic[MAX_TOPIC];
                    sscanf(buffer + 6, "%s", topic);

                    int found = 0;
                    for (int j = 0; j < clients[idx].sub_count; j++) {
                        if (strcmp(clients[idx].topics[j], topic) == 0) {
                            for (int k = j; k < clients[idx].sub_count - 1; k++) {
                                strcpy(clients[idx].topics[k], clients[idx].topics[k + 1]);
                            }
                            clients[idx].sub_count--;
                            found = 1;
                            send(sd, "Unsubscribed OK\n", 16, 0);
                            break;
                        }
                    }
                    if (!found) {
                        send(sd, "Topic not found\n", 16, 0);
                    }
                }
                else if (strncmp(buffer, "PUB ", 4) == 0) {
                    char topic[MAX_TOPIC];
                    char msg[BUFFER_SIZE];
                    if (sscanf(buffer + 4, "%s %[^\n]", topic, msg) == 2) {
                        broadcast(topic, msg);
                    } else {
                        send(sd, "Invalid PUB format\n", 19, 0);
                    }
                }
            }
        }
    }

    return 0;
}
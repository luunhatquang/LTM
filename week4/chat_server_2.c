#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<poll.h>

#define MAX_FDS 1024
#define BUFFER_SIZE 1024
#define PORT 9000

typedef struct {
    char id[50];
    int registered;
} ClientStatus;

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    listen(listener, 10);

    struct pollfd fds[MAX_FDS];

    ClientStatus status[MAX_FDS];

    for(int i = 0; i < MAX_FDS; i++) {
        fds[i].fd = -1;
        status[i].registered = 0;
    }

    fds[0].fd = listener;
    fds[0].events = POLLIN;
    int nfds = 1;

    printf("Chat Server running on port %d...\n", PORT);

    while(1) {
        int ret = poll(fds, nfds, -1);
        if(ret < 0) {
            perror("poll failed");
            break;
        }

        for(int i = 0; i < nfds; i++) {
            if(fds[i].revents && POLLIN) {

                if(fds[i].fd == listener) {

                    int client_fd = accept(listener, NULL, NULL);
                    if(nfds < MAX_FDS) {
                        fds[nfds].fd = client_fd;
                        fds[nfds].events = POLLIN;
                        status[nfds].registered = 0;
                        nfds++;
                        send(client_fd, "Nhap theo cu phap 'client_id: client_name':\n", 45, 0);
                    }
                    else {
                        close(client_fd);
                    }
                }
                else {
                    char buf[BUFFER_SIZE];
                    int n = recv(fds[i].fd, buf, sizeof(buf) - 1, 0);
                    if(n <= 0) {
                        close(fds[i].fd);
                        fds[i] = fds[nfds - 1];
                        status[i] = status[nfds - 1];
                        nfds--;
                        i--;
                    }
                    else {
                        buf[n] = '\0';
                        if(buf[n - 1] == '\n') buf[n - 1] = '\0';
                        if(status[i].registered == 0) {
                            char id[50], name[50];
                            if (sscanf(buf, "%[^:]: %s", id, name) == 2) {
                                strcpy(status[i].id, id);
                                status[i].registered = 1;
                                send(fds[i].fd, "Dang ky thanh cong!\n", 12, 0);
                            }
                            else {
                                send(fds[i].fd, "Sai cu phap!\n", 13, 0);
                            }
                        }
                        else {
                            time_t now = time(NULL);
                            struct tm *t = localtime(&now);
                            char time_str[30];
                            strftime(time_str, sizeof(time_str), "%Y/%m/%d %I:%M:%p", t);
                            char out_buf[BUFFER_SIZE + 100];
                            sprintf(out_buf, "%s %s: %s\n", time_str, status[i].id, buf);
                            for (int j = 1; j < nfds; j++) { // Gửi cho mọi người trừ listener
                                if (j != i && status[j].registered == 1) {
                                    send(fds[j].fd, out_buf, strlen(out_buf), 0);
                                }
                            }
                        }
                    }
                }
            }
        }

    }
    


}
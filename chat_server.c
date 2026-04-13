#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<time.h>

#define PORT 9000
#define BACKLOG 10
#define BUF_SIZE 2048

typedef struct 
{
    int fd;
    char id[64];
    int registered;
} Client;

Client clients[FD_SETSIZE];
int num_clients = 0;

void getCurrentTime(char *buffer) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 26, "%Y/%m/%d %I:%M:%S%p", timeinfo);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) {
        perror("socket");
        exit(1);

    } 
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if(listen(listener, BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server is listening on port %d...\n", PORT);
    fd_set readfds;
    int max_fd;

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        max_fd = listener; 

        for(int i = 0; i < num_clients; i++) {
            FD_SET(clients[i].fd, &readfds);
            if(clients[i].fd > max_fd) {
                max_fd = clients[i].fd;
            }
        }

        if(select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if(FD_ISSET(listener, &readfds)) {
            int client_fd = accept(listener, NULL, NULL);
            if(num_clients < FD_SETSIZE) {
                clients[num_clients].fd = client_fd;
                clients[num_clients].registered = 0;
                num_clients++;
                char *msg = "Vui lòng nhập tên theo cú pháp: client_id: client_name\n";
                send(client_fd, msg, strlen(msg),0);
            }
            else {
                close(client_fd);
            }
        }

        for(int i = 0; i < num_clients; i++) {
            if(FD_ISSET(clients[i].fd, &readfds)) {
                char buf[BUF_SIZE];
                int ret = recv(clients[i].fd, buf, BUF_SIZE - 1, 0);

                if(ret < 0) {
                    close(clients[i].fd);
                    clients[i] = clients[num_clients - 1];
                    num_clients--;
                    i--;
                    continue;
                }

                buf[ret] = '\0';
                if (buf[ret-1] == '\n') buf[ret-1] = '\0';
                if (buf[ret-2] == '\r') buf[ret-2] = '\0';

                if(clients[i].registered == 0) {
                    char id[64], name[64];

                    if(sscanf(buf, "client_id: %s", name) == 1) {
                        strcpy(clients[i].id, name);
                        clients[i].registered = 1;
                        char *success_msg = "Đăng nhập thành công! Bạn có thể bắt đầu chat.\n";
                        send(clients[i].fd, success_msg, strlen(success_msg), 0);
                    }

                    else {
                        char *fail_msg = "Sai cú pháp! Yêu cầu: client_id: client_name\n";
                        send(clients[i].fd, fail_msg, strlen(fail_msg), 0);
                    }
                }
                else {
                    char time_str[30];
                    getCurrentTime(time_str);

                    char send_buf[BUF_SIZE + 100];
                    sprintf(send_buf, "%s %s: %s\n", time_str, clients[i].id, buf);

                    for (int j = 0; j < num_clients; j++) {
                        if (clients[j].fd != clients[i].fd && clients[j].registered == 1) {
                            send(clients[j].fd, send_buf, strlen(send_buf), 0);
                        }
                    }
                }
            }
        }
    }
    close(listener);
    return 0;
    
}

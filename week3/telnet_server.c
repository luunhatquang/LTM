#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define PORT 9001
#define BUF_SIZE 2048

typedef struct {
    int fd;
    int state; 
    char username[64];
} Client;

Client clients[FD_SETSIZE];
int num_clients = 0;

int check_login(const char *user, const char *pass) {
    FILE *f = fopen("accounts.txt", "r");
    if (f == NULL) return 0;

    char db_user[64], db_pass[64];
    while (fscanf(f, "%s %s", db_user, db_pass) != EOF) {
        if (strcmp(user, db_user) == 0 && strcmp(pass, db_pass) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Telnet Server đang chạy trên cổng %d...\n", PORT);

    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        int max_fd = listener;

        for (int i = 0; i < num_clients; i++) {
            FD_SET(clients[i].fd, &readfds);
            if (clients[i].fd > max_fd) max_fd = clients[i].fd;
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(listener, &readfds)) {
            int client_fd = accept(listener, NULL, NULL);
            clients[num_clients].fd = client_fd;
            clients[num_clients].state = 0; 
            num_clients++;
            send(client_fd, "Username: ", 10, 0);
        }

        for (int i = 0; i < num_clients; i++) {
            if (FD_ISSET(clients[i].fd, &readfds)) {
                char buf[BUF_SIZE];
                int ret = recv(clients[i].fd, buf, BUF_SIZE - 1, 0);
                if (ret <= 0) {
                    close(clients[i].fd);
                    clients[i] = clients[num_clients - 1];
                    num_clients--; i--; continue;
                }

                buf[ret] = '\0';
                if (buf[ret-1] == '\n') buf[ret-1] = '\0';
                if (buf[ret-2] == '\r') buf[ret-2] = '\0';

                
                if (clients[i].state == 0) { 
                    strcpy(clients[i].username, buf);
                    clients[i].state = 1; 
                    send(clients[i].fd, "Password: ", 10, 0);
                } 
                else if (clients[i].state == 1) { 
                    if (check_login(clients[i].username, buf)) {
                        clients[i].state = 2;
                        send(clients[i].fd, "Login thành công. Nhập lệnh: \n", 31, 0);
                    } else {
                        send(clients[i].fd, "Sai rồi. Username: ", 19, 0);
                        clients[i].state = 0;
                    }
                } 
                else if (clients[i].state == 2) {
                    char cmd[BUF_SIZE + 20];
                    sprintf(cmd, "%s > out.txt 2>&1", buf); 
                    system(cmd);

                    FILE *f = fopen("out.txt", "r");
                    char line[BUF_SIZE];
                    while (fgets(line, sizeof(line), f)) {
                        send(clients[i].fd, line, strlen(line), 0);
                    }
                    fclose(f);
                    send(clients[i].fd, "\nCommand: ", 10, 0);
                }
            }
        }
    }
    return 0;
}
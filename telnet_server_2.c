#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#define PORT 9001
#define BUF_SIZE 2048

typedef struct {
    int fd;
    int state; 
    char username[64];
} Client;

Client clients[1024];
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

    struct pollfd fds[1024];
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    int nfds = 1;

    while (1) {
        poll(fds, nfds, -1);

        if (fds[0].revents & POLLIN) {
            int client_fd = accept(listener, NULL, NULL);
            fds[nfds].fd = client_fd;
            fds[nfds].events = POLLIN;
            clients[nfds-1].fd = client_fd;
            clients[nfds-1].state = 0; 
            nfds++;
            send(client_fd, "Username: ", 10, 0);
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                char buf[BUF_SIZE];
                int ret = recv(fds[i].fd, buf, BUF_SIZE - 1, 0);
                if (ret <= 0) {
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];
                    clients[i-1] = clients[nfds - 2];
                    nfds--; i--; continue;
                }

                buf[ret] = '\0';
                if (buf[ret-1] == '\n') buf[ret-1] = '\0';
                if (ret > 1 && buf[ret-2] == '\r') buf[ret-2] = '\0';

                if (clients[i-1].state == 0) { 
                    strcpy(clients[i-1].username, buf);
                    clients[i-1].state = 1; 
                    send(fds[i].fd, "Password: ", 10, 0);
                } 
                else if (clients[i-1].state == 1) { 
                    if (check_login(clients[i-1].username, buf)) {
                        clients[i-1].state = 2;
                        send(fds[i].fd, "Login thành công. Nhập lệnh: \n", 31, 0);
                    } else {
                        send(fds[i].fd, "Sai rồi. Username: ", 19, 0);
                        clients[i-1].state = 0;
                    }
                } 
                else if (clients[i-1].state == 2) {
                    char cmd[BUF_SIZE + 20];
                    sprintf(cmd, "%s > out.txt 2>&1", buf); 
                    system(cmd);
                    FILE *f = fopen("out.txt", "r");
                    char line[BUF_SIZE];
                    while (fgets(line, sizeof(line), f)) {
                        send(fds[i].fd, line, strlen(line), 0);
                    }
                    fclose(f);
                    send(fds[i].fd, "\nCommand: ", 10, 0);
                }
            }
        }
    }
    return 0;
}
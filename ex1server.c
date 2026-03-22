#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9006);
    int opt = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("SOCKET FAILED");
        exit(EXIT_FAILURE);
    }

    if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("BIND FAILED");
        exit(EXIT_FAILURE);
    }

    listen(listener, 5);
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(listener, (struct sockaddr*)&client_addr, &client_addr_len);
        char buf[2048];
        while(1) {
            int n = recv(client, buf, sizeof(buf),0);
            if (n <= 0) {
                printf("end connect");
                break;
            }
            buf[n] = '\0';
            printf("Client gửi: %s\n", buf);

        }
        close(client);
    }
    close(listener);
}
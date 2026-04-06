#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if(argc != 4) {
        printf("Use: %s <port_s> <ip_d> <port_d>\n", argv[0]);
        exit(1);
    }

    int port_s = atoi(argv[1]);
    char *ip_d = argv[2];
    int port_d = atoi(argv[3]);

    int sockfd;
    struct sockaddr_in local_addr, remote_addr;
    char buffer[BUF_SIZE];
    fd_set read_fds;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Cant not create socket");
        exit(1);
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(port_s);

    if(bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port_d); 
    if (inet_pton(AF_INET, ip_d, &remote_addr.sin_addr) <= 0) {
        printf("IP dich khong hop le!\n");
        exit(1);
    }

    printf("He thong UDP da sang sang (Port nhan: %d)\n", port_s);
    printf("Nhap tin nhan de gui den %s:%d", ip_d, port_d);

    while(1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sockfd, &read_fds);

        if(select(sockfd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Lỗi hàm select");
            break;
        }

        if(FD_ISSET(STDIN_FILENO, &read_fds)) {
            if(fgets(buffer, BUF_SIZE, stdin) != NULL) {
                sendto(sockfd, buffer, strlen(buffer), 0,(struct sockaddr*)&remote_addr, sizeof(remote_addr));
            }
        }
        if(FD_ISSET(sockfd, &read_fds)) {
            struct sockaddr_in src_addr;
            socklen_t addr_len = sizeof(src_addr);
            int len = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0 , (struct sockaddr*)&src_addr, &addr_len);

            if(len > 0) {
                buffer[len] = '\0';
                printf("[Nhan tu %s]: %s", inet_ntoa(src_addr.sin_addr), buffer);
            }
        }
    }
    close(sockfd);
    return 0;


}

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

// Hàm main: Client TCP kết nối đến server và gửi tin nhắn
int main(int argc, char *argv[]) {
    // Kiểm tra đối số dòng lệnh: IP và cổng
    if (argc != 3) {
        printf("Sử dụng: %s <Địa chỉ IP> <Cổng>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Tạo socket TCP
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client < 0) {
        perror("SOCKET FAILED");
        exit(EXIT_FAILURE);
    }

    // Thiết lập cấu trúc địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(server_ip);
    addr.sin_port = htons(server_port);

    // Kết nối đến server
    if(connect(client, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("CONNECT FAILED");
        exit(EXIT_FAILURE);
    }

    printf("Đã kết nối tới server %s:%d\n", server_ip, server_port);
    printf("Nhập tin nhắn (Gõ 'exit' để thoát):\n");

    char buf[2048];
    // Vòng lặp để gửi tin nhắn
    while(1) {
        printf("> ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) break;

        buf[strcspn(buf, "\n")] = '\0';
        if (strcmp(buf, "exit") == 0) break;

        // Gửi tin nhắn đến server
        int sent = send(client, buf, strlen(buf), 0);
        if (sent < 0) {
            perror("SEND FAILED");
            exit(EXIT_FAILURE);
        }
    }
    // Đóng socket
    close(client);
    printf("Đã đóng kết nối.\n");
    return 0;
}
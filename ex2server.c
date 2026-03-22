#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>

// Hàm main: Server TCP lắng nghe kết nối, gửi lời chào và ghi log tin nhắn
int main(int argc, char *argv[]) {
    // Kiểm tra đối số: cổng, tệp chào, tệp log
    if(argc != 4) {
        printf("Sử dụng: %s <cổng> <tệp_câu_chào> <tệp_lưu_nội_dung>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    char *greeting_file = argv[2];
    char *log_file = argv[3];

    // Tạo socket TCP
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Thiết lập địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // Cho phép tái sử dụng địa chỉ
    int opt = 1;
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("SOCKET FAILED");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    if(bind(listener, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("BIND FAILED");
        exit(EXIT_FAILURE);
    }
    // Lắng nghe kết nối
    listen(listener, 5);
    printf("Server đang đợi ở cổng %d...\n", port);
    // Vòng lặp chấp nhận kết nối
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
        
        if(client < 0) {
            continue;
        }
        printf("Kết nối mới từ: %s\n", inet_ntoa(client_addr.sin_addr));

        // Gửi lời chào từ tệp
        FILE *f_greet = fopen(greeting_file, "r");
        if (f_greet != NULL) {
            char greet_buf[2048];
            while(fgets(greet_buf, sizeof(greet_buf), f_greet) != NULL) {
                send(client, greet_buf, strlen(greet_buf), 0);
            }
            fclose(f_greet);
        }

        // Mở tệp log
        FILE *f_log = fopen(log_file, "a");
        if(f_log == NULL) {
            perror("Không thể mở tệp lưu nội dung");
            close(client);
            continue;
        }

        char buf[2048];
        // Nhận và ghi log tin nhắn
        while(1) {
            int n = recv(client, buf, sizeof(buf) - 1, 0);
            if(n <= 0) break;
            buf[n] = '\0';
            fprintf(f_log, "%s: %s\n", inet_ntoa(client_addr.sin_addr), buf);
            fflush(f_log);
            printf("Đã lưu dữ liệu từ %s\n", inet_ntoa(client_addr.sin_addr));
        }
        fclose(f_log);
        close(client);
    }
    close(listener);
}
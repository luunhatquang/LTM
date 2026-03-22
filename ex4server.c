#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<time.h>

// Cấu trúc thông tin sinh viên
struct sinhvien {
    char mssv[15];     
    char ho_ten[50];
    char ngay_sinh[20]; 
    float diem_tb;
};

// Hàm main: Server nhận thông tin sinh viên và ghi log
int main(int argc, char *argv[]) {
    // Kiểm tra đối số: cổng và tệp log
    if(argc != 3) {
        printf("Sử dụng: %s <Cổng> <File Log>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    char *log_filename = argv[2];
    // Thiết lập địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    // Tạo socket TCP
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Cho phép tái sử dụng địa chỉ
    int opt = 1;
    if(setsockopt(listener,SOL_SOCKET,SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("socket failed");
        return 1;
    }

    // Bind socket
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("Bind failed");
        return 1;
    }

    // Lắng nghe kết nối
    listen(listener, 5);
    printf("Server đang đợi ở cổng %d...\n", port);

    // Vòng lặp chấp nhận kết nối
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);

        if(client < 0) return 0;
        
        struct sinhvien sv;

        // Nhận thông tin sinh viên
        int n = recv(client, &sv, sizeof(sv),0);

        if(n > 0) {
            // Lấy thời gian hiện tại
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char time_str[26];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

            char *client_ip = inet_ntoa(client_addr.sin_addr);

            // Ghi log vào tệp
            FILE *f = fopen(log_filename, "a");
            fprintf(f, "%s %s %s %s %s %.2f\n", 
                    client_ip, 
                    time_str, 
                    sv.mssv, 
                    sv.ho_ten, 
                    sv.ngay_sinh, 
                    sv.diem_tb);
            fclose(f);
        }
        // Đóng socket client
        close(client);
    }
    // Đóng socket listener
    close(listener);
}
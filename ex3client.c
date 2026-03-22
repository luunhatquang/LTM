#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>

// Cấu trúc thông tin sinh viên
struct sinhvien {
    char mssv[15];     
    char ho_ten[50];
    char ngay_sinh[20]; 
    float diem_tb;
};

// Hàm main: Client nhập và gửi thông tin sinh viên đến server
int main(int argc, char *argv[]) {
    // Kiểm tra đối số: IP và cổng
    if (argc != 3) {
        printf("Sử dụng: %s <IP Server> <Cổng>\n", argv[0]);
        return 1;
    }
    // Tạo socket TCP
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Thiết lập địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    
    // Kết nối đến server
    if (connect(client, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
    struct sinhvien sv;
    
    // Nhập thông tin sinh viên
    printf("Nhập MSSV: ");
    fgets(sv.mssv, sizeof(sv.mssv), stdin);
    sv.mssv[strcspn(sv.mssv, "\n")] = 0;

    printf("Nhập Họ tên: ");
    fgets(sv.ho_ten, sizeof(sv.ho_ten), stdin);
    sv.ho_ten[strcspn(sv.ho_ten, "\n")] = 0;

    printf("Nhập Ngày sinh (dd/mm/yyyy): ");
    fgets(sv.ngay_sinh, sizeof(sv.ngay_sinh), stdin);
    sv.ngay_sinh[strcspn(sv.ngay_sinh, "\n")] = 0;

    printf("Nhập Điểm trung bình: ");
    scanf("%f", &sv.diem_tb);

    // Gửi thông tin sinh viên
    int sent = send(client, &sv, sizeof(sv), 0);
    
    if (sent > 0) {
        printf("--- Đã gửi thông tin sinh viên sang Server ---\n");
    }

    // Đóng socket
    close(client);
}
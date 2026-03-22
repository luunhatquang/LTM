#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>

struct sinhvien {
    char mssv[15];     
    char ho_ten[50];
    char ngay_sinh[20]; 
    float diem_tb;
};

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9007);

    int opt = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    if(bind(listener, (struct sockaddr*)&addr, sizeof(addr))){
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    listen(listener, 5);

    int client = accept(listener, NULL, NULL);

    struct sinhvien received_sv;

    int n = recv(client,&received_sv, sizeof(received_sv),0);
    if (n > 0) {
        printf("\n--- Thông tin sinh viên nhận được ---\n");
        printf("MSSV: %s\n", received_sv.mssv);
        printf("Họ tên: %s\n", received_sv.ho_ten);
        printf("Ngày sinh: %s\n", received_sv.ngay_sinh);
        printf("Điểm TB: %.2f\n", received_sv.diem_tb);
    }

    close(client);
    close(listener);

}
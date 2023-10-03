#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define MAX_PACKET_SIZE 2048
void err_sys(const char* x) 
{ 
    perror(x); 
    exit(1); 
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len;
    char serv_ip[11] = "127.0.0.11";
    char buffer[MAX_PACKET_SIZE];
    char challenge_id[256]; // 用于存储挑战ID
    char flag[256];         // 用于存储挑战的标志
    // 创建UDP套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
            err_sys("socket error");

    // 设置服务器地址
    // memset(&server_addr, 0, sizeof(server_addr));
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(10495);
    if (inet_pton(AF_INET, serv_ip, &(server_addr.sin_addr)) <= 0)
		err_sys("inet_pton error");

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		err_sys("connect error");
    
    // 发送 hello <id> 命令获取挑战ID
    const char *user_id = "110611008";
    sprintf(buffer, "hello %s", user_id);
    if(sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr))==-1) {
        printf("sending user id error\n");
    }
    else{
        printf("sending user id successfully\n");
    }
    

    // 接收挑战ID响应
    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL) == -1){
        printf("Not receive Server chanllege id\n");
    }else{
        printf("receive Server chanllege id");
        sscanf(buffer, "OK %s", challenge_id);
    }
    
    // 发送 chals <chals_id> 命令开始挑战
    sprintf(buffer, "chals %s", challenge_id);
    if(sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr))==-1) {
        printf("sending chanllege id error\n");
    }
    else{
        printf("sending chanllege id successfully\n");
    }
    // 用于标志解析的变量
    int flag_index = 0;
    int flag_started = 0;
    
    while(1){
        ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (recv_len == -1) {
            perror("recvfrom\n");
            printf("recvfrom connect error\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        // 分析数据包内容
        buffer[recv_len] = '\0';
        if (strstr(buffer, "BEGIN FLAG") != NULL) {
            flag_started = 1; // 标志开始
            continue;
        } else if (strstr(buffer, "END FLAG") != NULL) {
            break; // 标志结束
        }
        if (flag_started) {
            // 计算x-header-length并转换为ASCII字符
            int x_header_length = recv_len;
            char flag_char = (char)x_header_length;

            // 添加到标志中
            flag[flag_index++] = flag_char;
        }
    }
    flag[flag_index] = '\0';
    printf("Flag : %s\n", flag);
    close(sockfd);
    return 0;
}

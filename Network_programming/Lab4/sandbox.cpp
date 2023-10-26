#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include<utility>

#define MAXLINE 30000
#define HMAZE 21
#define WMAZE 79

using namespace std;
const char* SERV_IP = "140.113.213.213";
const int PORT = 10315;
// string maze[HMAZE];

void err_quit(const char* msg);
void print_maze();

int main(int argc, char **argv)
{
	int					sockfd;
    int n;
	struct sockaddr_in	servaddr;
    char buffer[MAXLINE];
    char sendline[MAXLINE];

	// if (argc != 2)
	// 	err_quit("usage: tcpcli <SERV PORT>");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERV_IP, &servaddr.sin_addr);

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror("Connnect ");
        close(sockfd);
        return -1;
    }
    printf("connect success!\n");
}
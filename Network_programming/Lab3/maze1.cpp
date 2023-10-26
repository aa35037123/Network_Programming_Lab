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

#define MAXLINE 1000
#define HMAZE 7
#define WMAZE 12

using namespace std;
void err_quit(const char* msg) {
    perror(msg);
    exit(1);
}


ssize_t writen(int fd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;
    ptr = (const char *)vptr;
    nleft = n;
    while(nleft > 0) {
        if((nwritten = write(fd, ptr, nleft) <= 0)) {
            if(nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}


ssize_t readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;
    ptr = (char *) vptr;
    for (n = 1; n < maxlen; n ++) {
        if (rc = read(fd, &c, 1) == 1) { /*read 1 byte a time*/
            *ptr++ = c;
            if (c == '\n') 
                // continue;
                break;
        }
        else if (rc == 0) {
            *ptr = 0; /*add NULL to the end of ptr*/
            return n-1; /*cause the final char isn't read*/
        }
        else {
            return -1;
        }
    }
    *ptr = 0;/*add NULL to the end of ptr*/
    return n;
}

void str_cli(FILE *fp, int sockfd) {
    char    sendline[MAXLINE], recvline[MAXLINE];

    while (fgets(sendline, MAXLINE, fp) != NULL) {
 
        writen(sockfd, sendline, strlen(sendline));
 
        if (readline(sockfd, recvline, MAXLINE) == 0)
            err_quit("str_cli: server terminated prematurely");
 
        fputs(recvline, stdout);
    }
}

void get_server_message(FILE *fp, int sockfd) {
    char    sendline[MAXLINE], recvline[MAXLINE];

    while(readline(sockfd, recvline, MAXLINE) != 0) {
        fputs(recvline, stdout);
        if (fgets(sendline, MAXLINE, fp) == NULL) {
            printf("client close connect\n");
            return;
        }
        writen(sockfd, sendline, strlen(sendline));
    }
}

int main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;
    const char* SERV_IP = "140.113.213.213";
    char buffer[MAXLINE];
    char sendline[MAXLINE];
    string maze[HMAZE];
	if (argc != 2)
		err_quit("usage: tcpcli <SERV PORT>");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	inet_pton(AF_INET, SERV_IP, &servaddr.sin_addr);

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror("Connnect ");
        close(sockfd);
        return -1;
    }
    printf("connect success!\n");

	// str_cli(stdin, sockfd);		/* do it all */
    // get_server_message(stdin, sockfd);
    read(sockfd, buffer, MAXLINE);
    std::cout << buffer << '\n';
    read(sockfd, buffer, MAXLINE);
    std::cout << buffer << '\n';

    /*get maze string*/
    std::string buffer_str = buffer;
    std::string searchStr = "Note2: Total number of movements must be less than 16 steps.";
    size_t found = buffer_str.find(searchStr);
    if(found != string::npos) {
        // cout << "Found string " << buffer_str[found+searchStr.length()+2];
        int current_idx =  found+searchStr.length()+2;
        // cout << "first sub string: "<<buffer_str.substr(found, current_idx);
        for (int i = 0; i < HMAZE; i++) {
            
            // maze[i] = buffer_str.substr(current_idx, current_idx+WMAZE);
            maze[i] = buffer_str.substr(current_idx, WMAZE);
            current_idx += WMAZE;
            cout << maze[i];
        }
    }
    pair<int, int> E_coord;
    pair<int, int> S_coord;
    for (int i = 0; i < HMAZE; i++) {
        for (int j = 0; j < WMAZE; j++) {
            if(maze[i][j] == 'E') {
                E_coord.first = i;
                E_coord.second = j;
            }
            else if(maze[i][j] == '*') {
                S_coord.first = i;
                S_coord.second = j;
            }
        }
    }
    
    int y_move = E_coord.first - S_coord.first;
    int x_move = E_coord.second - S_coord.second;
    bzero(&sendline, sizeof(sendline));
    int ans_cnt = 0;
    if(x_move <= 0){ /*A*/
        for (int i = 0; i < x_move*-1; i++) {
            sendline[ans_cnt++] = 'A';
        }
    }
    else{/*D*/
        for (int i = 0; i < x_move; i++) {
            sendline[ans_cnt++] = 'D';
        }
    }
    if(y_move <= 0){ /*S*/
        for (int i = 0; i < y_move*-1; i++) {
            sendline[ans_cnt++] = 'W';
        }
    }
    else{/*W*/
        for (int i = 0; i < y_move; i++) {
            sendline[ans_cnt++] = 'S';
        }
    }
    sendline[ans_cnt] = '\n';
    cout << sendline;
    write(sockfd, sendline, MAXLINE);
    bzero(&buffer, sizeof(buffer));
    read(sockfd, buffer, MAXLINE);
    cout << buffer << '\n';
    read(sockfd, buffer, MAXLINE);
    cout << buffer << '\n';
    // std::cout << "found str: " << found << "\n"; 
    // while(1) {
    //     bzero(&sendline, sizeof(sendline));
    //     fgets(sendline, MAXLINE, stdin);
    //     std::cout << "send message: " << sendline;
    //     write(sockfd, sendline, MAXLINE);
    //     bzero(&buffer, sizeof(buffer));
    //     read(sockfd, buffer, MAXLINE);
    //     std::cout << buffer << '\n';
    //     std::string str(buffer);
    //     std::string success_message = str.substr(0, 6);
    //     if(success_message == "BINGO!")
    //         break;
        
    // }
    return 0;
}

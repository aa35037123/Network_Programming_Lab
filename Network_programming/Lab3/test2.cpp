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
const int PORT = 10302;
char maze[HMAZE+1][WMAZE+1];
// string maze[HMAZE];

void err_quit(const char* msg);
void print_maze();
void dfs(int row, int column, string route);

bool found = false;
string ans = "";

int main(int argc, char **argv)
{
	int					sockfd;
    int n;
	struct sockaddr_in	servaddr;
    const char* SERV_IP = "140.113.213.213";
    char buffer[MAXLINE];
    char buffer1[MAXLINE];
    char buffer2[MAXLINE];
    char sendline[MAXLINE];

	// if (argc != 2)
	// 	err_quit("usage: tcpcli <SERV PORT>");
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv;
    tv.tv_sec = 3;/*timeout in second*/
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	
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

    bzero(&buffer, sizeof(buffer));
    int nbytes = recv(sockfd, buffer, sizeof(buffer), 0);
    if (nbytes <= 0) {
        close(sockfd);
        printf("server closed connection.\n");
    }
    cout << "recv: " << buffer << "\n\n";
    bzero(&buffer1, sizeof(buffer1));
    nbytes = recv(sockfd, buffer1, sizeof(buffer1), 0);
    if (nbytes <= 0) {
        close(sockfd);
        printf("server closed connection.\n");
    }
    cout << "recv: " << buffer1 << "\n\n";
    bzero(&buffer2, sizeof(buffer2));
    nbytes = recv(sockfd, buffer2, sizeof(buffer2), 0);
    if (nbytes <= 0) {
        close(sockfd);
        printf("server closed connection.\n");
    }
    cout << "recv: " << buffer2 << "\n\n";
    string maze_buffer = buffer1;
    maze_buffer += buffer2;
    // bzero(&buffer, sizeof(buffer));
    // nbytes = recv(sockfd, buffer, sizeof(buffer), 0);
    // if (nbytes <= 0) {
    //     close(sockfd);
    //     printf("server closed connection.\n");
    // }
    // cout << "recv: " << buffer << "\n\n";
    cout << "concate: " << maze_buffer << '\n';
    int row = 0;
    /*each for loop i + 1 will automatically flush the '\n' char*/
    for(int i = 0; i < maze_buffer.length(); i++){
        if(maze_buffer[i] == '#'){
            strncpy(maze[row], &maze_buffer[i], WMAZE);
            i += WMAZE;
            row++;
        }
        if(row >= 21){
            cout << "filled maze!\n";
            break;
        }
    }
    bool find_start = false;
    int x_start = 0, y_start = 0;
    for (int i = 0; i < HMAZE && !find_start; i++) {
        for (int j=0; j < WMAZE && !find_start; j ++) {
            if(maze[i][j] == '*'){
                x_start = i;
                y_start = j;
                // cout << "x_start: "<< x_start <<", y_start: " << y_start << '\n';
                dfs(x_start, y_start, "");
                find_start = true;
            }
        }
    }
    ans += '\n';
    cout << "ans: " << ans;
    bzero(&sendline, sizeof(sendline));
    strncpy(sendline, ans.c_str(), ans.length());
    write(sockfd, sendline, ans.length());

    bzero(&buffer, sizeof(buffer));
    read(sockfd, buffer, MAXLINE);
    cout << buffer << "\n\n";

    bzero(&buffer, sizeof(buffer));
    read(sockfd, buffer, MAXLINE);
    cout << buffer << "\n\n";
    // print_maze();
}

void err_quit(const char* msg) {
    perror(msg);
    exit(1);
}

void print_maze(){
    cout << "Maze: \n";
    for(int r = 0; r < 21; r++){
        for(int c = 0; c < 79; c++){
            cout << maze[r][c];
        }
        cout << endl;
    }
    cout << endl;
}

void dfs(int row, int column, string route) {
    // cout << "Current route: " << route << '\n'; 
    if(found) return;
    if(maze[row][column] == 'E') {
        found = true;
        ans = route;
        return;
    }
    else if(maze[row][column] == '#' || maze[row][column] == '$') {
        return;
    }
    maze[row][column] = '$'; /*means that visited*/
    dfs(row, column +1, route + 'D');/*right*/
    dfs(row+1, column, route + 'S');/*down*/
    dfs(row, column-1, route + 'A');/*left*/
    dfs(row-1, column, route + 'W');/*up*/
}

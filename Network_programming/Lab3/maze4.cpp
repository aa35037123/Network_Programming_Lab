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

#define MAXLINE 90000
#define HMAZE 7
#define WMAZE 11
#define inf 10000
using namespace std;
const int PORT = 10304;
char maze[HMAZE+10][WMAZE+10];
char buffer[MAXLINE];
// string maze[HMAZE];

void err_quit(const char* msg);
void receive(bool discard);
int find_map(char* ch);
void query(string q);
void print_maze();
void dfs(int from, string route) ;
void update_viewport();

int sockfd;
bool found = false;
string ans = "";

int main(int argc, char **argv)
{
    int n;
	struct sockaddr_in	servaddr;
    const char* SERV_IP = "140.113.213.213";
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
    receive(false);receive(false);
    update_viewport();
    dfs(-1, "");
}
void update_viewport(){
    int row = 0;
    for (int i = 0; i < sizeof(buffer); i ++) {
        if(find_map(&buffer[i]) != -inf){
            strncpy(&maze[row][0], &buffer[i+3], 11);
            row++; i+=11;
            if(row >= 7) break;
        }
    }
}
/*fromDire means the opposite to coming way of dfs
    ex: go 'W', then my fromDire is 'S'(the order of 'S' in go is 2)*/
int fromDirec[4] = {2, 3, 0, 1}; 
string go[4] = {"W", "A", "S", "D"};
string back[4] = {"S", "D", "W", "A"};
int coord[4][2] = {{2, 5}, {3, 4}, {4, 5}, {3, 6}};
/*start point: (3, 5)*/
void dfs(int from, string route) {
    if(found) return;
    cout << "Current route: " << route << '\n';
    for(int i = 0; i < 4; i++) {
        if(i == from) continue;/*don't waste time to go back*/
        /*prelook*/
        if(maze[coord[i][0]][coord[i][1]] == 'E'){
            found = true;
            query(go[i]);receive(false);receive(false);receive(false);
            int tmp; cin >> tmp;
            return;
        }
        if(maze[coord[i][0]][coord[i][1]] != '#') {
            query(go[i]); receive(false); receive(true);
            update_viewport();
            dfs(fromDirec[i], route + go[i]);
            // if(!found){
            //     query(back[i]); receive(false); receive(true);
            //     update_viewport();
            // }
            query(back[i]); receive(false); receive(true);
            update_viewport();

        }
    }
}
// void dfs(int from, string route){ // right, left, up, down
//     if(found) return;
//     cout << route << "\n\n";
//     int fromDirec[4] = {1, 0, 3, 2};
//     int direction[4][2] = {{3, 6}, {3, 4}, {2, 5}, {4, 5}};
//     string go[4] = {"D", "A", "W", "S"};
//     string back[4] = {"A", "D", "S", "W"};
//     for(int i = 0; i < 4; i++){
//         if(i == from) continue;
//         if(maze[direction[i][0]][direction[i][1]] == 'E'){
//             found = true;
//             query(go[i]); receive(false); receive(false); receive(false);
//             int tmp; cin >> tmp;
//             return;
//         }
//         if(maze[direction[i][0]][direction[i][1]] != '#'){
//             query(go[i]); receive(false); receive(true);
//             update_viewport();
//             dfs(fromDirec[i], route + go[i]);
//             query(back[i]); receive(false); receive(true);
//             update_viewport();
//         }
//     }
    
// } // current position {3, 5}
void query(string q) {
    q += '\n';
    bzero(&buffer, sizeof(buffer));
    strncpy(buffer, q.c_str(), q.length());
    int n = write(sockfd, buffer, q.length());
    cout << "query: " << buffer << "\n";
    
}
void receive(bool discard) {   
    char tmp[MAXLINE]; 
    if(discard){
        bzero(&tmp, sizeof(tmp));
        int n = read(sockfd, tmp, sizeof(tmp));
        cout << "=====discard======\n" << tmp <<'\n';
        return;
    }
    bzero(&buffer, sizeof(buffer));
    int n = read(sockfd, buffer, sizeof(buffer));
    cout << buffer << "\n\n";
}
/*find_map transfer char to integer or find some specific char*/
int find_map(char* ch){
    if(*ch >= '0' && *ch <= '9' && *(ch+1) == ':'){
        if(*(ch-1) == 'e') return -inf;
        int result = 0;
        result += (*ch) - '0';
        if(*(ch-1) == '-') result = -result;
        if(*(ch-1) >= '0' && *(ch-1) <= '9') result += 10 * ((*(ch-1) - '0'));
        if(*(ch-2) >= '0' && *(ch-2) <= '9') result += 100 * ((*(ch-2) - '0'));
        return result;
    } else {
        return -inf;
    }
}
void print_maze(){
    cout << "Maze: \n";
    for(int r = 0; r < HMAZE; r++){
        for(int c = 0; c < WMAZE; c++){
            cout << maze[r][c];
        }
        cout << endl;
    }
    cout << endl;
}
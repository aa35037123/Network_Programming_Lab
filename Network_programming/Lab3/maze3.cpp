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
#define HMAZE 110
#define WMAZE 110
#define inf 10000
using namespace std;
const int PORT = 10303;
char maze[HMAZE+10][WMAZE+10];
char buffer[MAXLINE];
// string maze[HMAZE];

void err_quit(const char* msg);
void receive(bool discard);
int find_map(char* ch);
void query(string q);
void print_maze();
void dfs(int row, int column, string route);

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
    receive(true);receive(false);
    cout << "$$ buffer: " << buffer << '\n';
    /*parse buffer*/
    string q = "";
    int row = 0, space = 0;/*space means that the space between 'number: ' and map*/
    /*move view port to top*/
    for(int i = 0; i < sizeof(buffer); i++) {
        // if return value != -inf, means that buffer goes to front of row
        if((row = find_map(&buffer[i])) != -inf) {   
            cout << "$$row: " << row << '\n';
            // pull the view port to the middle
            while(buffer[i + 3 + space] == ' '){
                space++;
            }
            if(row) q += string(row, 'I');
            else q += string(-row, 'K');
            if(space) q += string(space, 'L');/*string(n, char) mean that repeat char {n} times*/
            query(q); receive(false); receive(true);
            break;
        }
    }

    // move view port to left
    // if space != 0, means that view port already at very left
    // cout << "current buffer: "<<buffer << '\n';
    for(int i = 0; i < sizeof(buffer) / sizeof(char) && !space; i++){
        if(find_map(&buffer[i]) != -inf){
            // cout << "I'm in for loop\n";
            if(buffer[i+3] != ' '){
                q = "J";
                query(q); receive(false); receive(true);
                i = 0; // data in the buffer is renewed
            }
        }
    }
    
    if(!space){
        /*go right 1 unit because view port has 1 left offset caused by above for loop*/
        query("L");receive(false);receive(true);
    }
    /*size of view view port: 7 * 11*/
    for(int row=0; row < HMAZE; row += 7){
        for(int col=0; col < WMAZE; col += 11){
            int view_row = 0;
            // cout << "current buffer: "<<buffer << '\n';
            for(int i = 0; i < sizeof(buffer); i++){
                if(find_map(&buffer[i]) != -inf){
                    strncpy(&maze[row+view_row][col], &buffer[i+3], 11);
                    view_row++; i+=11;
                    if(view_row >= 7) break;
                }
            }
            /*move view to right*/ 
            q = string(11, 'L');
            query(q); receive(false); receive(true); 
        }
        /*move view down and left*/ 
        q = string(7, 'K') + string(99+11, 'J');
        query(q); receive(false); receive(true); 
    }
    // print_maze();
    bool find_start = false;
    int x_start = 0, y_start = 0;
    for (int i = 0; i < HMAZE && !find_start; i++) {
        for (int j=0; j < WMAZE && !find_start; j ++) {
            if(maze[i][j] == '*'){
                x_start = i;
                y_start = j;
                find_start = true;
            }
        }
    }
    cout << "x_start: "<< x_start <<", y_start: " << y_start << '\n';
    dfs(x_start, y_start, "");
    // cout << "ans: " << ans;
    query(ans);receive(false);receive(false);receive(false);
}
void dfs(int row, int column, string route) {
    if(found) return;
    if(maze[row][column] == 'E') {
        found = true;
        ans = route;
        return;
    }
    else if(maze[row][column] == '#') {
        return;
    }
    maze[row][column] = '#'; /*means that visited*/
    dfs(row, column +1, route + 'D');/*right*/
    dfs(row+1, column, route + 'S');/*down*/
    dfs(row, column-1, route + 'A');/*left*/
    dfs(row-1, column, route + 'W');/*up*/
}
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
    cout << buffer << "\n\n\n\n";
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
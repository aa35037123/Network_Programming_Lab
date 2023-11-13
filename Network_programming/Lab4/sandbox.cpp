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
// public server
// const char* SERV_IP = "140.113.213.213";
// const int PORT = 10314;
char buffer[MAXLINE];
// // internal 
const char*SERV_IP = "172.21.0.4";
const int PORT = 10001;
string server_str = "http://inp.zoolab.org:10314";
struct sockaddr_in	servaddr;
int sockfd;
void err_quit(const char* msg);
void print_maze();
void Receive();
void Send(string query); 
bool is_connect();
int main(int argc, char **argv)
{

    int n;
	
    string request;
	// if (argc != 2)
	// 	err_quit("usage: tcpcli <SERV PORT>");
    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
    // convert numeric(SERV_IP) to format that network accept
	inet_pton(AF_INET, SERV_IP, &servaddr.sin_addr);
	// catch when connect failed
    if(!is_connect()){
        return -1;
    }
    
    // \r\n\r\n is the format of HTTP
    // request = "GET /logs HTTP/1.1\r\n\r\n";
    // Send(request);
    // Receive();Receive();

    request = "GET /otp?name=110611008 HTTP/1.1\r\n\r\n";
    Send(request);
    Receive();Receive();
    string otp = buffer;
    // check connect everytime send and receive file
    if(!is_connect()){
        return -1;
    }
    request = "POST /upload HTTP/1.1\r\n";
    
    request = "POST /upload HTTP/1.1\r\n";
    request += "Host: ";
    request += SERV_IP;
    request += ":";
    request += to_string(PORT);
    request += "\r\nConnection: keep-alive\r\n";
    // request += "\r\nContent-Type: application/octet-stream\r\n";
    request += "Content-Length: " + std::to_string(otp.length() + 182) + "\r\n";
    request += "Cache-Control: max-age=0\r\n";
    request += "Upgrade-Insecure-Requests: 1\r\n";
    // request += "Origin: http://inp.zoolab.org:10314\r\n";
    request += "Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryyHib3LZwTSIuHAFn\r\n";
    request += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36\r\n";
    request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n";
    // request += "Referer: http://inp.zoolab.org:10314/upload\r\n";
    request += "Accept-Encoding: gzip, deflate\r\n";
    request += "Accept-Language: en-US,en;q=0.9,zh-TW;q=0.8,zh;q=0.7,ja;q=0.6\r\n\r\n";
    request += "------WebKitFormBoundaryyHib3LZwTSIuHAFn\r\n";
    request += "Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n";
    request += "Content-Type: text/plain\r\n\r\n";
    request += otp;
    // request += "\r\n\r\n";
    request += "\r\n------WebKitFormBoundaryyHib3LZwTSIuHAFn--\r\n";
    Send(request);
    Receive();Receive();
    return 0;
}
//WebKitFormBoundaryyHib3LZwTSIuHAFn
bool is_connect(){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        cout << "error: create socket";
        return false;
    }

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror("Connnect ");
        close(sockfd);
        return false;
    }
    printf("connect success!\n");
    return true;
}
void Receive() {   
    // int nbytes = recv(sockfd, buffer, sizeof(buffer), 0);
    bzero(&buffer, sizeof(buffer));
    int nbytes = read(sockfd, buffer, sizeof(buffer));
    if (nbytes <= 0) {
        // close(sockfd);
        printf("reading data error\n");
    }
    cout << "====receive====\n" << buffer << "\n\n";
}
void Send(string query){
    bzero(&buffer, sizeof(buffer));
    strncpy(buffer, query.c_str(), query.length());
    int nbytes = write(sockfd, buffer, query.length());
    if (nbytes <= 0) {
        // close(sockfd);
        printf("sending data error\n");
    }
    cout << "====send====\n";
    cout << query << "\n\n";
}
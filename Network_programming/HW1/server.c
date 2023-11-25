#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define errquit(m)	{ perror(m); exit(-1); }
#define MAXLINE 1500000
#define DMAXLINE 3000000

static int port_http = 80;
static int port_https = 443;
static const char *docroot = "/html";

const char *get_file_extension(const char *filename) {
	// strrchr: search for the last time '.' appear
    const char *dot = strrchr(filename, '.');
	// Check if there is no dot in the file_name or if it is the first character.
    if (!dot || dot == filename) {
        // If there is no dot or it's the first character, there's no extension.
		return "";
    }
	// Return the portion of the file_name after the dot (the file extension).
    return dot + 1;
}

void url_decode(char *str) {
    char *in = str;
    char *out = str;

    while (*in) {
		// printf("before:\n");
		// printf("in: %s, out: %s\n", in, out);
		// printf("str: %s\n", str);
		// out and str is at the same index each time 
		// each time read 1 byte(2 hex char) from in
		// and put the hex value into out 
        if (*in == '%' && isxdigit(*(in + 1)) && isxdigit(*(in + 2))) {
            // Convert percent-encoded value to character
            sscanf(in + 1, "%2hhx", out);
            in += 2;
        } else {
			// If the current character is not part of a percent-encoded sequence
			// copy the character from in to out directly.
            *out = *in;
        }
		
        in++;
        out++;
		// printf("after:\n");
		// printf("in: %s, out: %s\n", in, out);
		// printf("str: %s\n", str);
    }

    *out = '\0';
}

const char *get_mime_type(const char *file_extension){
	// + '/' because we need to add the tailing slash
	// strcasecmp: if two string are equal, ignore case
	if (strcasecmp(file_extension, "html") == 0 || strcasecmp(file_extension, "htm/") == 0) {
        return "text/html";
    } else if (strcasecmp(file_extension, "txt") == 0) {
        return "text/plain;charset=utf-8";
    } else if (strcasecmp(file_extension, "jpg") == 0 || strcasecmp(file_extension, "jpeg/") == 0) {
        return "image/jpeg";
    } else if (strcasecmp(file_extension, "png") == 0) {
        return "image/png";
    } else if(strcasecmp(file_extension, "mp3") == 0){
		return "audio/mpeg";
	}
	else {
        return "application/octet-stream";
    }
}
void removeFirstChar(char *str) {
	int len = strlen(str);
    // Check if the string is not empty
	for(int i = 0; i < len-1; i++){
		str[i] = str[i+1];
	}
	str[len-1] = '\0';
    // if (str != NULL && str[0] != '\0') {
    //     // Shift all characters to the left by one position
    //     memmove(str, str + 1, strlen(str));
    // }
	// str[len-1] = '\0';
}

void *handle_http_client(void *arg){
	int connfd = *(int*)arg;
	char buf[MAXLINE];
	char send_line[DMAXLINE];
	char file_extension[1024];
	// receive request data from client and store into buffer
    ssize_t bytes_received = recv(connfd, buf, MAXLINE, 0);
	// ssize_t bytes_received = read(connfd, buf, MAXLINE);
	if (bytes_received > 0){
		// bzero(&buf, sizeof(buf));
		// snprintf(buf, MAXLINE,"HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nhello\n");
		// send(connfd, buf, strlen(buf), 0);
		// bzero(&send_line, sizeof(send_line));
		// snprintf(send_line, DMAXLINE,"HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n%s\n", buf);
		// send(connfd, send_line, strlen(send_line), 0);
		struct stat file_stat;
		char *request = buf;
		url_decode(request);
		// bzero(&send_line, sizeof(send_line));
		// snprintf(send_line, MAXLINE, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\ndecode path: %s\n", request);
		// send(connfd, send_line, strlen(send_line), 0);
		
		char *method = strtok(request, " ");
		char *path = strtok(NULL, " ");
		char *protocol = strtok(NULL, " ");
		// // if method is not "GET", return 0
		if(strcmp(method, "GET") != 0){
			bzero(&send_line, sizeof(send_line));
			snprintf(send_line, MAXLINE, "HTTP/1.1 501 Not Implemented\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n\r\n"
		"501 Not Implemented\n"
		);
			send(connfd, send_line, strlen(send_line), 0);
			close(connfd);
			free(arg); /*arg is the int pointer of connfd */
			return NULL;
		}
		// if path is root, then redirect it to index.html
		// it means we visit root of web
		if(strcmp(path, "/") == 0){
			// find if index.html exist in root
			strcpy(path, "/index.html/");	
			// check if index.html exist in root
			// if(stat(path, &file_stat) != 0){/*not exist*/
			// 	bzero(&send_line, sizeof(send_line));
			// 	snprintf(send_line, MAXLINE, "HTTP/1.0 403 Forbidden\r\nContent-Type: text/plain\r\n\r\n403 Forbidden");
			// 	send(connfd, send_line, strlen(send_line), 0);
			// 	close(connfd);
			// 	return NULL;
			// }
			// else{
			// 	// bzero(&send_line, sizeof(send_line));
			// 	// snprintf(send_line, MAXLINE, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nfind html in: %s\n", path);
			// 	// send(connfd, send_line, strlen(send_line), 0);
			// 	strcpy(path, "/index.html"); // if index.html in data folder, then convert index.html to origin form 	
			// }
		}
		
		

		/*remove first '/' char from path*/
		
		
		// bzero(&send_line, sizeof(send_line));
		// snprintf(send_line, MAXLINE, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nexten: %s\n", file_extension);
		// send(connfd, send_line, strlen(send_line), 0);
		removeFirstChar(path);/*remove the '/' in front of every path, o.w it can not be found*/
		
		// bzero(&send_line, sizeof(send_line));
		// snprintf(send_line, MAXLINE*2,"HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\rnew_path:%s\n", path);
		// send(connfd, send_line, strlen(send_line), 0);
		

		int path_len = strlen(path);
		// remove the tailing slash in the end of path, o.w it will check fault
		bool tailing_slash = false;
		if(path[path_len-1] == '/'){
			tailing_slash = true;
			path[path_len-1] = '\0';
			// write 301 code
		}
		// bzero(&send_line, sizeof(send_line));
		// snprintf(send_line, MAXLINE*2,"new_path:%s\n", path);
		// send(connfd, send_line, strlen(send_line), 0);

		// check for filename is end with "/"
		strcpy(file_extension, get_file_extension(path));

		char response[DMAXLINE];
		size_t response_len;
		
		bool isDir = false;
		
		// check if file can be found
		if(stat(path, &file_stat) != 0) {
			bzero(&send_line, sizeof(send_line));
			snprintf(send_line, MAXLINE, "HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found");
			send(connfd, send_line, strlen(send_line), 0);
			close(connfd);
			free(arg); /*arg is the int pointer of connfd */
			return NULL;
		}
		if (S_ISDIR(file_stat.st_mode)) {
			isDir = true;
		} 
		// if path is dir, and not end with tailing slash, need to send 301 code
		// after client receive 301 code, client browser will make a new request 
		// to the specified location 
		if(isDir && !tailing_slash){
			bzero(&send_line, sizeof(send_line));
			snprintf(send_line, MAXLINE, "HTTP/1.0 301 Moved Permanently\r\nLocation: /%s/\r\n\r\n", path);
			send(connfd, send_line, strlen(send_line), 0);
			close(connfd);
			free(arg); /*arg is the int pointer of connfd */
			return NULL;
		}
		// has right request format, but the directory is forbidden
		if(isDir && tailing_slash){/*check if index.html exist in this dir*/
			strcat(path, "/index.html");
			// if index file can not be found
			if(stat(path, &file_stat) != 0) {
				bzero(&send_line, sizeof(send_line));
				snprintf(send_line, MAXLINE, "HTTP/1.0 403 Forbidden\r\nContent-Type: text/plain\r\n\r\n403 Forbidden");
				send(connfd, send_line, strlen(send_line), 0);
			}else{
				strcpy(file_extension, get_file_extension(path));
				const char *mime_type = get_mime_type(file_extension);
				FILE *fd;
				if ((fd = fopen(path,"rb")) != NULL){
					bzero(&send_line, sizeof(send_line));
					snprintf(send_line, MAXLINE,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n",
							mime_type);
					send(connfd, send_line, strlen(send_line), 0);
					ssize_t bytes_read;
					bzero(&send_line, sizeof(send_line));
					while((bytes_read = fread(send_line, 1, MAXLINE, fd))>0){
						send(connfd, send_line, bytes_read, 0);
						bzero(&send_line, sizeof(send_line));
					}
				}
			}
			close(connfd);
			free(arg); /*arg is the int pointer of connfd */
			return NULL;
		}
		/*=============================FILE READ================================*/
		// build_http_response(file_path, file_extension, response, &response_len, connfd);
		// build HTTP header
		
    	const char *mime_type = get_mime_type(file_extension);
		
		FILE *fd;
		if ((fd = fopen(path,"rb")) != NULL){
			bzero(&send_line, sizeof(send_line));
			snprintf(send_line, MAXLINE,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n",
					mime_type);
			send(connfd, send_line, strlen(send_line), 0);
			ssize_t bytes_read;
			bzero(&send_line, sizeof(send_line));
			while((bytes_read = fread(send_line, 1, MAXLINE, fd))>0){
				send(connfd, send_line, bytes_read, 0);
				bzero(&send_line, sizeof(send_line));
			}
		}
		fclose(fd);
		// close(file_fd);
		// free(request);
		
	}
	close(connfd);
	free(arg); /*arg is the int pointer of connfd */
	return NULL;
}

int main(int argc, char *argv[]) {
	int listenfd_http, listenfd_https;
	struct sockaddr_in sin_http, sin_https;

	if(argc > 1) {
		port_http  = strtol(argv[1], NULL, 0); 
	}
	if(argc > 2) { 
		if((docroot = strdup(argv[2])) == NULL) errquit("strdup"); 
	}
	if(argc > 3) { 
		port_https = strtol(argv[3], NULL, 0); 
	}
	 /* 使用 /html 當網站根目錄 */
    if(chdir(docroot) == -1){ 
        printf("ERROR: Can't Change to directory %s\n",docroot);
        exit(-1);
    }
	/*===================HTTP=====================*/
	if((listenfd_http = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) errquit("socket");

	do {
		int v = 1;
		setsockopt(listenfd_http, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));
	} while(0);

	bzero(&sin_http, sizeof(sin_http));
	sin_http.sin_family = AF_INET;
	sin_http.sin_port = htons(port_http);
	sin_http.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listenfd_http, (struct sockaddr*) &sin_http, sizeof(sin_http)) < 0) errquit("bind");
	if(listen(listenfd_http, SOMAXCONN) < 0) errquit("listen");
	/*===================HTTPS=====================*/
	
	
	do {
		int *connfd_http = malloc(sizeof(int));
		int *connfd_https = malloc(sizeof(int));
		struct sockaddr_in csin;
		socklen_t csinlen = sizeof(csin);
		pthread_t thread_id_http, thread_id_https;
		// FILE *fp;
		// char buf[4096];
		/*wait for client connect*/
		/*===================HTTP=====================*/
		if((*connfd_http= accept(listenfd_http, (struct sockaddr*) &csin, &csinlen)) >= 0) {
			pthread_create(&thread_id_http, NULL, handle_http_client, (void *)connfd_http);
			//  detach thread: system releasing the resources associated with the thread once it finishes its execution. 
			pthread_detach(thread_id_http);
			// continue;
		}
		else{
			// perror("accept");
			continue;
		}



	} while(1);
	close(listenfd_http);
	close(listenfd_https);
	return 0;
}

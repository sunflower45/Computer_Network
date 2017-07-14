#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>

#define BUFSIZE 2048

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock;
    int clnt_sock;
    char buffer[BUFSIZE];
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;
    int  read_len;
    char res_index[] = "HTTP/1.1 200 OK\r\nServer:Linux Web Server \r\nContent-length:123\r\nContent-type:text/html\r\n\r\n";
    char res_query[] = "HTTP/1.1 200 OK\r\nServer:Linux Web Server \r\nContent-length:384\r\nContent-type:text/html\r\n\r\n";
    char res_nf[] = "HTTP/1.1 404 Not Found\r\nServer:Linux Web Server \r\nContent-length:0\r\nContent-type:text/html\r\n\r\n";
    char res_post1[] = "HTTP/1.1 200 OK\r\nServer:Linux Web Server \r\n Content-length:";
    char res_post2[] = "\r\nContent_type:text/html\r\n\r\n";
    if(argc!=2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
        error_handling("bind() error");

    if(listen(serv_sock, 20)==-1)
        error_handling("listen() error");

    while(1){
	FILE* clnt_read;
	FILE* clnt_write;
	FILE* send_file;
	char file_buf[BUFSIZE]="\0";
	char method[10]="\0";
	char file_name[30]="\0";
	char data[100]="\0";
	char* read_line="\0";
	int i = 0;
	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	printf("Connection Request : %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
	if(clnt_sock==-1)
		error_handling("accept() error");
	
    	memset(buffer, 0, BUFSIZE);
	read_len = recv(clnt_sock, buffer, BUFSIZE, 0);
	if(read_len > 0) {
		printf("RECV : %s", buffer);
	}
	clnt_read = fdopen(clnt_sock, "r");
	clnt_write = fdopen(dup(clnt_sock), "w");
	read_line = strtok(buffer," /");
	strcpy(method, read_line);
	read_line = strtok(NULL, "HTTP/");
	strcpy(file_name, read_line);
	printf("method : %s\n", method);
	fclose(clnt_read);
	if(strcmp(method, "GET")==0){
		if(strcmp(file_name, " ")==0 || strcmp(file_name,"index.html ")==0){
			printf("filename : %s\n", file_name);
			send_file = fopen("index.html", "r");
			//printf("\nsend file : %p", send_file);
			if(send_file == NULL) {
				printf("send file open error\n");
				fclose(clnt_write);
				return;
			}
			printf("response : %s", res_index);
			fputs(res_index, clnt_write);
		}
		else if(strcmp(file_name, "query.html ")==0){
			printf("filename : %s\n", file_name);
			send_file = fopen("query.html", "r");
			//printf("\nsend file : %p", send_file);
			if(send_file == NULL) {
				printf("send file open error\n");
				fclose(clnt_write);
				return;
			}
			printf("response : %s", res_query);
			fputs(res_query, clnt_write);
		}
		else {
			printf("filename : %s\n", file_name);
			fputs(res_nf, clnt_write);
		}
		while(fgets(file_buf, BUFSIZE, send_file)!=NULL) {
			fputs(file_buf, clnt_write);
			fflush(clnt_write);
		}
		fflush(clnt_write);
		fclose(clnt_write);
	}
	else if(strcmp(method, "POST")==0){
		while(read_line = strtok(NULL, "\r\n")){
			if(strstr(read_line, "name=")!=NULL){
				strcpy(data, strstr(read_line, "name="));
			} else {
				strcpy(data,"\0");
			}
		}
		printf("\ndata : %s\n", data);
		while(i < 100) {
			if(data[i] == '\0') {
				break;
			}
			i++;
		}
		printf("\ndata size : %d\n", i);
		char temp[128];
		i = i + 36;
		sprintf(temp, "%d", i);
		if(strcmp(file_name, "response.html ")==0){
			fputs(res_post1, clnt_write);
			fputs(temp, clnt_write);
			fputs(res_post2, clnt_write);
			fputs("<html><body><h2>", clnt_write);
			fputs(data, clnt_write);
			fputs("</h2></body></html>", clnt_write);
		}
		fflush(clnt_write);
		fclose(clnt_write);
	}
	else {
		printf("ONLY GET AND POST\n");
		fclose(clnt_read);
		fclose(clnt_write);
		return;
	}
    	//close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

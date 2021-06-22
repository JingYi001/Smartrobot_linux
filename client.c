#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include<string.h>

#define SRV_PORT 6666
#define INPUT_SIZE 81;

void err(const char* str){
    perror(str);
    exit(1);
}

int main(){
	int afd, ret;
	char buf[BUFSIZ];
    char input[BUFSIZ];

    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port =  htons(SRV_PORT);
    inet_pton(AF_INET, "1.15.231.208", &srv_addr.sin_addr.s_addr);

    afd = socket(AF_INET, SOCK_STREAM, 0);
   	if(afd < 0){ err("socket error"); }

    ret = connect(afd, (struct sockaddr*) &srv_addr, sizeof(srv_addr));
    while(ret < 0){ ret = connect(afd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)); }

    while(1){
        printf("我:");
        gets(input);

        write(afd, input, strlen(input));
        memset(buf,0,sizeof(buf));
        ret = read(afd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, strlen(buf));
        printf("\n");
    }
	write(afd, "\r\n", 2);
    close(afd);

    return 0;
}

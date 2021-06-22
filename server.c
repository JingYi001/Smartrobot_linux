#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define SRV_PORT 6666

char res[512];

void err(const char* str){
    perror(str);
    exit(1);
}

//填写你自己申请的APIkey 
const char* apikey="58417f2a42f7093faf385743918cce0e";
int repResolve( char *rep, char *recmd ){
    //p q接受匹配后的起始地址 
    char *p = strstr( rep, "reply" );
    //char *q = strstr( rep, "rough" );
    if(p){
        // recommend": " r到"一共13个字符 
        p = p + 8;
        // 地名" 一直匹配到引号的结束
        while (!((*p) == '\"')){
            //字符串复制 
            *recmd++ = *p++;
        }
        //字符串末尾补0
        *recmd = '\0';
    }
   /* if(q){
        q += 9;
        while (!((*q) == '\"')){
            *rough++ = *q++;
        } 
        *rough = '\0';
    } */
    return(0);
}

int reply(char *input){
    char buff[4096] = {'\0'};
    char recmd[512] = {'\0'};

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("create socket error!\n");
        exit( -1 );
    }
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons( 80 );
    inet_pton( AF_INET, "220.181.136.79", &serveraddr.sin_addr.s_addr );
    /* inet_pton(AF_INET, "apis.map.qq.com", &serveraddr.sin_addr.s_addr); */
    if(connect(sockfd,(struct sockaddr*) &serveraddr, sizeof(serveraddr))<0){
        printf("connect error!\n");
        exit(-2);
    }
    memset(buff,0,sizeof(buff));
    snprintf( buff, sizeof(buff)-1,
        "GET /txapi/robot/index?key=%s&question=%s HTTP/1.1\r\n"
        "Host: api.tianapi.com\r\n"
        "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n" "\r\n",
        apikey,input );
    if(write( sockfd, buff, strlen(buff)) != strlen(buff)){
        return(-1);
    }
    memset(buff, 0, sizeof(buff));
    size_t size;
    if((size = read( sockfd, buff, sizeof(buff))) < 0 ){
        return(-1);
    }
    /*步骤4：关闭socket*/
    //printf("\nTCP响应长度: %d\n",size);
    //printf( "HTTP响应报文长度: %d\n",strlen(buff));
    //printf( "HTTP响应报文:\n%s\n", buff );
    //printf("HTTP响应报文:\n%.*s\n",1000,buff);
    repResolve( buff, recmd );
    
    strcpy(res, "");
    strcpy(res, recmd);
    //printf( "当前坐标: \n%s %s\n", argv[1], argv[2] );
    close( sockfd );
    //printf( "回复: %s\n", recmd );
    //printf( "\t%s\n", rough );*/
    
}


int main(){
    pid_t pid;

    int lfd, afd, ret;
    char buf[BUFSIZ], client_IP[1024];

    struct sockaddr_in srv_addr, clt_addr;
    socklen_t clt_addr_len;

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SRV_PORT);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd < 0){ err("socket error"); }

    ret = bind(lfd, (struct sockaddr*) &srv_addr, sizeof(srv_addr));
    if(ret < 0){ err("bind error"); }

    ret = listen(lfd, 128);
    if(ret < 0){ err("listen error"); }

    clt_addr_len = sizeof(clt_addr);
    while(1){
    	afd = accept(lfd, (struct sockaddr*) &clt_addr, &clt_addr_len);
    	if(afd < 0){ err("accept error"); }
	    // father continue wait next client
	    pid = fork();
	    if(pid < 0){ err("fork error"); sleep(1); continue; }
	    if(pid > 0){ continue; }

	    printf("Son pid %d listen\n", getpid());
        printf("Client IP: %s, port: %d\n",
            inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
            ntohs(clt_addr.sin_port));

        int done = 0;
	    while(!done){

            memset(buf,0,sizeof(buf));
            ret = read(afd, buf, sizeof(buf));
            
            write(STDOUT_FILENO, buf, ret);
	        if(buf[0]=='\r'){
	    	    close(lfd);
    		    close(afd);
	    	    printf("Client IP: %s, port: %d, exit\n",
		        inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
                    ntohs(clt_addr.sin_port));
		        exit(0);
	        }
            for(int i=0; i<ret; ++i){
		        if(buf[i] == '\r'){ done=1; break; }
            }
            reply(buf);
	        write(afd, res, strlen(res));
            printf("\n");
        }

        close(lfd);
        close(afd);
    }

    return 0;
}

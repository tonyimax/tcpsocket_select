//
// Created by linux on 2021/2/3.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc,char* argv[]){

    unsigned short port = 8000;
    if (argc<0){
        port=atoi(argv[1]);
    }

    int sockfd = 0;
    sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sockfd<0){
        perror("socket");
        exit(-1);
    }

    struct sockaddr_in srvaddr;
    bzero(&srvaddr,sizeof (srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int errlog = connect(sockfd,(struct sockaddr*)&srvaddr, sizeof(srvaddr));
    if (errlog !=0)
    {
        perror("connect");
        close(sockfd);
        exit(-1);
    }

    while (true){
        char send_buf[512] = "HelloWorld\n";
        char redv_buf[512] = "";
        printf("send:");
        fgets(send_buf, sizeof(send_buf),stdin);
        send_buf[strlen(send_buf)-1]=0;
        send(sockfd,send_buf,sizeof (send_buf),0);

        int i;
        i=recv(sockfd,redv_buf, sizeof(redv_buf),0);
        printf("i=%d\r\n",i);
        printf("recv:%s\r\n",redv_buf);
    }

    close(sockfd);
    return 0;
}

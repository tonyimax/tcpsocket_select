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

int main(int argc,char* argv[])
{
    unsigned short port = 8000;
    if (argc>1)
    {
        port=atoi(argv[1]);
    }

    pid_t pid;
    pid = fork();

    if (pid<0){
        perror("fork error");
    }else if (pid==0){
        printf("child proc\r\n");
    }else{
        printf("parent proc\r\n");
    }

    int sockfd = 0;
    sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if (sockfd<0)
    {
        perror("socket");
        exit(-1);
    }

    struct sockaddr_in addr;
    bzero(&addr,sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    int errlog = bind(sockfd,(struct sockaddr*)&addr, sizeof(addr));

    if (errlog !=0)
    {
        perror("bind");
        close(sockfd);
        exit(-1);
    }

    errlog = listen(sockfd,10);
    if (errlog !=0)
    {
        perror("listen");
        close(sockfd);
        exit(-1);
    }

    printf("listen client @port=%d\r\n",port);

    struct sockaddr_in client_addr;
    char ipstr[INET_ADDRSTRLEN]="";
    socklen_t client_addr_len = sizeof(client_addr);
    int connfd;

    connfd = accept(sockfd,(struct sockaddr*)&client_addr,&client_addr_len);
    if (connfd<0){
        perror("accept");
        exit(-1);
    }

    inet_ntop(AF_INET,&client_addr.sin_addr,ipstr,INET_ADDRSTRLEN),
    printf("client ip=%s,port=%d\r\n",ipstr,client_addr.sin_port);


    while (true)
    {
        char recv_buf[2048]="";
        if (recv(connfd,recv_buf,sizeof (recv_buf),0)>0){
            printf("recv data:\r\n");
            printf("%s\r\n",recv_buf);
            char send_buf[512] = "";
            printf("send:");
            fgets(send_buf, sizeof(send_buf),stdin);//read stdin data
            send_buf[strlen(send_buf)-1]=0;
            send(connfd,send_buf, sizeof(send_buf),0);
        }

    }

    close(connfd);
    printf("client closeed\r\n");

    close(sockfd);
    return 0;
}

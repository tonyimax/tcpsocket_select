//
// Created by linux on 2021/2/4.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
int main(int argc,char*argv[])
{
    int i,j,n,maxi;//这里i=0,j=0,n=0,maxi=0
    int nready,clients[FD_SETSIZE];
    int maxfd,connfd,sfd,sockfd;
    char buf[512],str[INET_ADDRSTRLEN];

    struct sockaddr_in client_addr,server_addr;
    socklen_t client_addr_len;
    fd_set rset,allset;
    unsigned short port = 8000;//监听端口
    //参数判断
    if (argc>1)
    {
        port=atoi(argv[1]);
    }
    //创建socket
    sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sockfd<0)
    {
        perror("创建套接字出错");
        exit(-1);
    }
    //初始化服务绑定信息
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    //绑定
    int errlog = bind(sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr));
    if (errlog !=0)
    {
        perror("绑定出错");
        close(sockfd);
        exit(-1);
    }
    //监听
    errlog = listen(sockfd,10);
    if (errlog!=0)
    {
        perror("监听");
        close(sockfd);
        exit(-1);
    }
    printf("正在等待客户连接，监听端口：%d\r\n",port);

    maxfd = sockfd;//最大文件描述
    maxi= -1;//索引

    //遍历文件描述集合
    for ( i = 0; i < FD_SETSIZE; ++i) {
        clients[i] = -1;//默认初始化为－1
    }

    FD_ZERO(&allset);//置零fd_set
    FD_SET(sockfd,&allset);//添加sockfd到fd_set

    //循环调用select函数
    while (true)
    {
        rset = allset;
        nready = select(maxfd+1,&rset,NULL,NULL,NULL);
        if (nready<0){perror("select函数调用出错");}
        //判断sockfd是否置位，如置位代表有可读数据，即有新的客户端连入
        if (FD_ISSET(sockfd,&rset))
        {
            client_addr_len = sizeof(client_addr);//客户端地长度
            connfd = accept(sockfd,(struct sockaddr*)&client_addr,&client_addr_len);//接受客户端连接
            //打印客户端IP地址和端口
            printf("接受客户端%s端口：%d连接,",inet_ntop(AF_INET,&client_addr.sin_addr,str, sizeof(str)), ntohs(client_addr.sin_port));
            //遍历集合保存已连接客户端到集合，代表上前已连接客户端数量
            for ( i = 0; i < FD_SETSIZE; ++i) {
                if (clients[i]<0)
                {
                    clients[i] = connfd;
                    printf("客户端索引i:-->%d,",i);
                    break;
                }
            }
            printf("已连接客户端数量:%d\n",i+1);
            //超出最大集合范围
            if (i==FD_SETSIZE)
            {
                fputs("太多客户端了\n",stderr);
                exit(1);
            }
            FD_SET(connfd,&allset);//保存已连接客户端到集合
            if (connfd>maxfd){maxfd=connfd;}//保持maxfd是最新连接的FD
            if (i>maxi){maxi = i;}//保持maxi是clients[]中最后一个元素,maxi保存了已连接客户端数量
            if (--nready==0){continue;}//即使是最后一个也继续 1  --1 = 0
        }
        //遍历当前已连接客户端
        for ( i = 0; i <= maxi; ++i) {
            //当前元素为-1，未置位
            if ((sfd=clients[i])<0){continue;}
            //如果被置位，工资有数据可读，rset是select函数中传入的read_set,代表读套接字描述符号
            if (FD_ISSET(sfd,&rset))
            {
                //读取socket数据，如果长度为0代表客户端连接已关闭
                if ((n=read(sfd,buf,sizeof (buf)))==0)
                {
                    printf("客户端连接已关闭,");
                    close(sfd);//关闭socket
                    FD_CLR(sfd,&allset);//从集合中移除socket fd
                    clients[i] = -1;//重置临时保存的sockfd
                    maxi-=1;//客户端断开连接时减去一个连接数
                    printf("当前总连接客户端%d\n",maxi+1);
                }else if (n>0) //有数据可读
                {
                    printf("接收的数据:%s\r\n",buf);
                    //printf("输入数据回车发送给客户端:");
                    //fgets(buf, sizeof(buf),stdin);//标准输入流输入
                    //buf[strlen(buf)-1]=0;//字符串结尾
                    //send(sfd,buf,strlen(buf),0);//发送
                }
                if (--nready==0){break;}//最后一个也跳出 1  --1 = 0
            }
        }
    }
    close(sockfd);//关闭监听socket
    return 0;
}

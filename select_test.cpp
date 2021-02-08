//
// Created by linux on 2021/2/5.
//
#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main(int argc,char* argv[])
{
    cout<<"linux select网络模型演示"<<endl;

    //变量声明
    int i=0;//客户端连接索引
    int nready=0;
    int maxi=-1;

    unsigned short port = 8000;//服务监听端口
    int maxListen = FD_SETSIZE;//客户端最大连接数量

    //创建socket
    int socket_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (socket_fd<0)
    {
        cout<<"创建socket失败"<<endl;
        exit(-1);
    }

    //初始化服务socket地址结果
    struct sockaddr_in server_addr;
    server_addr.sin_port = htons(port);//端口
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);//任何地址
    server_addr.sin_family = AF_INET;//协议家族

    //绑定socket地址
    int result =bind(socket_fd,(struct sockaddr*)&server_addr, sizeof(server_addr));
    if (result!=0)
    {
        cout<<"绑定socket失败"<<endl;
        close(socket_fd);
        exit(-1);
    }

    //监听socket
    result = listen(socket_fd,maxListen);
    if (result!=0)
    {
        cout<<"监听socket失败"<<endl;
        close(socket_fd);
        exit(-1);
    }

    //保存连接的客户端的fd,默认-1代表未保存
    int socket_clients[FD_SETSIZE];//最大1024个连接
    for (i = 0; i < FD_SETSIZE; ++i) {
        socket_clients[i]=-1;
    }

    fd_set socket_set;//保存客户端已连接成功的connect_fd -->for save
    fd_set read_set;//正在读取的fd   --->for read

    FD_ZERO(&socket_set);//置零
    FD_ZERO(&read_set);//置零

    FD_SET(socket_fd,&socket_set);//将监听socket添加到集合

    struct sockaddr_in client_addr;//客户端地址
    socklen_t client_addr_len;//客户端地址长度
    int connect_fd;//客户端连接文件描述
    char str[INET_ADDRSTRLEN];//保存IP地址

    int maxfd=socket_fd;//maxfd保存了服务监听socket fd号
    cout<<"服务监听fd:"<<socket_fd<<endl;
    cout<<"进入循环前maxfd:"<<maxfd<<endl;
    int cur_sock_fd;//当前有数据可读socket
    int nbyteread=0;//socket读取长度
    char buf[512]="";//保存socket读取数据

    while (true)
    {
        read_set = socket_set;
        //执行select函数,如果有可读socket操作，read_set集合包括读socket描述符
        nready=select(maxfd+1,&read_set,NULL,NULL,NULL);
        if (nready<0){perror("select function call error");}
        //read_set有文件可读
        if (FD_ISSET(socket_fd,&read_set))
        {
            //客户端地长度
            client_addr_len = sizeof(client_addr);
            //接受客户端连接
            connect_fd = accept(socket_fd,
                                (struct sockaddr*)&client_addr,
                                        &client_addr_len);
            //打印客户端IP地址和端口
            printf("接受客户端%s端口：%d连接,",
                   inet_ntop(AF_INET,&client_addr.sin_addr,str, sizeof(str)),
                   ntohs(client_addr.sin_port));

            for ( i = 0; i < FD_SETSIZE; ++i) {
                //如果socket_clients[i]还没保存connect_fd
                if (socket_clients[i]<0)
                {
                    //保存connect_fd到socket_clients[i]
                    socket_clients[i] = connect_fd;
                    cout<<"客户端索引i:"<<i<<endl;
                    break;//退出循环，此时i为当前的已连接客户端数量
                }
            }

            //i=0~1023
            if (i==FD_SETSIZE)
            {
                fputs("太多客户端了\n",stderr);
                exit(1);
            }

            //客户端已连接成功，保存connect_fd到socket_set
            FD_SET(connect_fd,&socket_set);

            if (connect_fd>maxfd){maxfd=connect_fd;}//保持maxfd是最新连接的FD
            if (i>maxi){maxi=i;}//保存当前客户端连接数量
            cout<<"maxi:->"<<maxi<<",maxfd:->"<<maxfd<<",客户端新连接connect_fd:"<<connect_fd<<endl;
            //打印socket_clients的元素
            for (int i = 0; i <=maxi; ++i) {
                cout<<"socket_clients["<<i<<"]->"<<socket_clients[i]<<endl;
            }
        }

        //遍历已连接的客户端
        for ( i = 0; i <= maxi; ++i) {
           cur_sock_fd = socket_clients[i];
           //当前socket有数据可读
            if (FD_ISSET(cur_sock_fd,&read_set))
            {
                //读取socket数据
                nbyteread =  read(cur_sock_fd,buf, sizeof(buf));
                //长度为0代表客户端关闭
                if (nbyteread == 0)
                {
                    cout<<"客户端"<<cur_sock_fd<<"连接已关闭"<<endl;
                    close(cur_sock_fd);//关闭客户端socket
                    //移除cur_sock_fd在socket_set中的置位
                    FD_CLR(cur_sock_fd,&socket_set);
                    socket_clients[i] = -1;//复位为未保存数据状态
                    for (int i = 0; i <=maxi; ++i) {
                        //只显示连接客户端
                        if (socket_clients[i]>0)
                        {
                            cout<<"socket_clients["<<i<<"]->"<<socket_clients[i]<<endl;
                        }
                    }
                } else if (nbyteread>0)//有数据可读
                {
                    //打印客户端发送的数据
                    cout<<"客户端"<<cur_sock_fd<<"发送的数据:"<<buf<<endl;
                }
            }
        }
    }
    close(socket_fd);//关闭服务socket
    return 0;
}
#include <stdio.h>
#include <iostream>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <vector>


#define MAXSIZE 1024

typedef struct sockid{
    int id;
    int client_fd;
} sockid;

typedef struct Arg{
    int fd;
    struct sockaddr_in addr;
    std::vector<sockid> Mq;
} Arg;


void Usage()
{
    printf("Usage:./server port\n");
}

//http发来的报文：app id close/open
//mcu发来的报文： mcu id connect

int getline(int client_fd, char* buf, char* source, char* id,  char* directive)
{
    ssize_t read_size = read(client_fd, buf, 1024);

    if(read_size == 0)
    {
        close(client_fd);
        return -1;
    }

    size_t i = 0;
    size_t j = 0;
    while(!isspace(buf[i]))
    {
        printf("source[%lu] = %c\n", j, source[j]);
        source[j++] = buf[i++];
    }
    source[j] = '\0';
    printf("source = %s\n", source);

    //走到这儿，将客户端确定
    while(isspace(buf[i]))
    {
        i++;
    }
    j = 0;
    while(!isspace(buf[i]))
    {
        directive[j++] = buf[i++];
    }
    directive[j] = '\0';
    printf("directive = %s\n", directive);

    //走到这儿读到命令
    while(isspace(buf[i]))
    {
        i++;
    }
    j = 0;
    while(buf[i] != '\0')
    {
        id[j++] = buf[i++];
    }
    id[j] = '\0';
    printf("id = %s\n", id);
    //读到id
    return 1;
}

int findSockId(int id, std::vector<sockid> mq)
{
    size_t i = 0;
    for(; i < mq.size(); i++)
    {
        if(mq[i].id == id)
        {
            break;
        }
    }
    if(i >= mq.size())
    {
        return -1;
    }
    return mq[i].client_fd;
}

void ProcessRequest(int client_fd, std::vector<sockid> mq){
    //tcp ok
    printf("程序走到这里3\n");
    char source[MAXSIZE/4] = {0};
    char directive[MAXSIZE/4] = {0};
    char buf[MAXSIZE] = {0};
    char id[MAXSIZE/4] = {0};

    getline(client_fd, buf, source, id,  directive);

    printf("%s:%s:%s\n", source, directive, id);

    if(strcasecmp(source, "mcu") == 0)
    {
        //如果是mcu，
        //1.需要将它维护起来
        //2.发送一个响应让其亮一下
        sockid sd;
        sd.client_fd = client_fd;
        sd.id = atoi(id);
        mq.push_back(sd);
        char respond_mcu[] = "tcp ok";
        send(client_fd, respond_mcu, sizeof(respond_mcu), 0);
    }
    else if(strcasecmp(source, "app") == 0)
    {
        printf("程序走到发送给app客户端循环里\n");
        //如果是app,
        //1.需要响应一个接收成功报文
        //2.需要根据id找到mcu的fd
        char respond[] = "tcp ok";
        send(client_fd, respond, sizeof(respond)-1, 0);
        printf("程序走到发送给app客户端\n");
        int mcu_fd = findSockId(atoi(id), mq);
        send(mcu_fd, directive, sizeof(directive), 0);
    }
}


void* CreateWorker(void* ptr )
{
    Arg* arg = (Arg*)ptr;
    printf("程序走到这里2\n");
    ProcessRequest(arg->fd, arg->Mq);
    free(arg);
    return NULL;
}

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        Usage();
        return -1;
    }
    std::vector<sockid> mq;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY; 
    //创建套接字
    int fd = socket(AF_INET,SOCK_STREAM, 0);
    if(fd < 0)
    {
        perror("socket");
        return -2;
    }
    //绑定套接字
    int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret < 0)
    {
        perror("bind");
        return -3;
    }
    //监听套接字
    ret = listen(fd, 10); 
    if(ret < 0)
    {
        perror("listen");
        return -4;
    }
    //接收套接字
    for(;;)
    {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(fd, (struct sockaddr*)&client_fd, &len);
        if(client_fd < 0)
        {
            perror("accept");
            continue;
        }

        
        char buf_ip[1024];                                                                                         
        buf_ip[0] = 0;
        if(inet_ntop(AF_INET, &client_addr.sin_addr, buf_ip, sizeof(buf_ip)) == NULL)
        {
            perror("inet_ntop");
            return 4;
        }
        printf("get a new connect : [%s | %d]\n", buf_ip, ntohs(client_addr.sin_port));

        pthread_t tid;
        Arg* arg = (Arg*)malloc(sizeof(Arg));
        arg->fd = client_fd;
        arg->addr = client_addr;
        arg->Mq = mq;
        printf("程序走到这里1\n");
        pthread_create(&tid, NULL, CreateWorker, (void*)arg);
        pthread_detach(tid);
    }
    return 0;
}

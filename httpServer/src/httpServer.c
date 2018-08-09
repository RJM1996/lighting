
#include "httpServer.h"

// 获取报头的每一行
// 要把 \r -> \n || \r\n -> \n
int get_line(int sock, char line[], int size)
{
    int i = 0;
    char ch = 'a';
    while(i < size - 1 && ch != '\n')
    {
        // ssize_t recv(int sockfd, void *buf, size_t len, int flags);
        ssize_t size = recv(sock, &ch, 1, 0); // 一次读一个字符
        if(size <= 0)
        {
            // 出错或者连接中断
            return -1;
        }
        else
        {
            // 继续判断
            if(ch == '\r')
            {
                // 窥探下一个字符
                // MSG_PEEK 窥看外来消息, 只看一下, 不读走
                size = recv(sock, &ch, 1, MSG_PEEK);
                if(size <= 0)
                {
                    return -1;
                }
                else
                {
                    if(ch == '\n')
                    {
                        // 如果接下来的一个字符是 '\n'
                        // 那么就把它读上来
                        size = recv(sock, &ch, 1, 0);
                        if(size <= 0)
                        {
                            return -1;
                        }
                    }
                    else
                    {
                        // 如果不是说明这一行已经完了, 把'\r'变为'\n'
                        ch = '\n';
                    }
                }
            }
            line[i++] = ch;
        }
    }
    line[i] = '\0';
    return i;
}

void echo_error(int sock)
{
    char buf[MAX_SIZE/4];
    memset(buf, 0, sizeof(buf));

    sprintf(buf, "HTTP/1.0 404 Not Found\r\n");
    send(sock, buf, strlen(buf), 0);

    send(sock, blank_line, strlen(blank_line), 0);

    const char* _404_path = "webroot/error_code/404/404.html";
    int fd = open(_404_path, O_RDONLY);
    if(fd == -1)
    {
        printf("404_open\n");
        perror("open");
        return ;
    }
    struct stat st;
    stat(_404_path, &st);
    // ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
    // 在内核中, 两个文件描述符之间直接进行读写, 效率高
    sendfile(sock, fd, NULL, st.st_size);
    close(fd);
}

// 返回状态信息
void status_response(int sock, int status_code)
{
    handle_hander(sock);
    switch(status_code)
    {
        case 404:
            echo_error(sock);
            break;
        case 503:
            break;
        default:
            break;
    }
}

// 响应
int echo_www(int sock, const char* resource_path, int size)
{
    int fd = open(resource_path, O_RDONLY);
    if(fd < 0)
    {
        printf("www_open\n");
        perror("open");
        return 404;
    }

    // ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    send(sock, status_line, strlen(status_line), 0);
    char len_buf[MAX_SIZE/4] = {0};
    // content_length = "Content-Length: %u\r\n"
    sprintf(len_buf, "Content-Length: %u\r\n", size);
    send(sock, len_buf, strlen(len_buf), 0);
    send(sock, blank_line, strlen(blank_line), 0);
    // ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
    // 在内核中, 两个文件描述符之间直接进行读写, 效率高
    sendfile(sock, fd, NULL, size);
    close(fd);
    return 200;
}

void handle_hander(int sock)
{
    char buf[1024];
    int ret = -1;
    do
    {
        ret = get_line(sock, buf, sizeof(buf));
    } while(ret > 0 && strcmp(buf, "\n") != 0);
}

static int exe_cgi(int sock, char* method, char* resource_path, char* query_string)
{
    // printf("运行 CGI 程序 ... \n");

    // 这里的父子进程通信方式选择匿名管道和环境变量
    // 环境变量父子进程都可以看见
    int content_length = -1;
    char method_env[MAX_SIZE/10];
    char query_string_env[MAX_SIZE];
    char content_length_env[MAX_SIZE/10];

    // 如果是 get 方法, 就清理掉剩余的报头
    if(strcasecmp(method, "GET") == 0)
    {
        handle_hander(sock);
    }
    else
    {
        // 如果是 post 方法
        // 因为参数在正文中, 所以首先要知道Content-Length
        char buf[1024];
        int ret = -1;
        do
        {
            ret = get_line(sock, buf, sizeof(buf));
            if(ret > 0 && strncasecmp(buf, "Content-Length: ", 16) == 0)
            {
                content_length = atoi(&buf[16]);
            }
        } while(ret > 0 && strcmp(buf, "\n") != 0);

        if(content_length == -1)
        {
            return 404;
        }
    }

    // 发送状态行和空行
    send(sock, status_line, strlen(status_line), 0);
    send(sock, blank_line, strlen(blank_line), 0);

    // 创建 2 个管道, 用于父子进程双向通信
    int input[2];
    int output[2];
    // int pipe(int pipefd[2]);
    if(pipe(input) < 0 || pipe(output) < 0)
    {
        perror("pipe");
        return 404;
    }
    //printf("input: %d %d\n", input[0], input[1]);
    //printf("output: %d %d\n", output[0], output[1]);

    pid_t pid = fork();
    if(pid < 0)
    {
        perror("fork");
        return 404;
    }
    else if(pid == 0)
    {
        // child
        close(input[1]);
        close(output[0]);
        sprintf(method_env, "METHOD=%s", method);
        putenv(method_env);
        if(strcasecmp(method, "GET") == 0)
        {
            sprintf(query_string_env, "QUERY_STRING=%s", query_string);
            putenv(query_string_env);
        }
        else
        {
            sprintf(content_length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(content_length_env);
        }

        int dup2_ret1;
        int dup2_ret2;
        if((dup2_ret1 = dup2(input[0], 0)) == -1 || (dup2_ret2 =  dup2(output[1], 1)) == -1)
        {
            perror("dup2");
            return 404;
        }
        //printf("dup2_ret1: %d, input[0]: %d\n", dup2_ret1, input[0]);
        //printf("dup2_ret2: %d, output[1]: %d\n", dup2_ret2, output[1]);

        // 到达程序替换
        int ret = execl(resource_path, resource_path, NULL);
        if(ret == -1)
        {
            perror("execl");
            return 404;
        }
        exit(1);
    }
    else
    {
        close(input[0]);
        close(output[1]);
        int i = 0;
        char ch = '\0';
        if(strcasecmp(method, "POST") == 0)
        {
            for(; i < content_length; i++)
            {
                recv(sock, &ch, 1, 0);
                send(input[1], &ch, 1, 0);
            }
        }
        ch = '\0';
        while(read(output[0], &ch, 1))
        {
            //printf("output[0] : %d\n", output[0]);
            //printf("input[1] : %d\n", input[1]);
            send(sock, &ch, 1, 0); 
        }

        int ret = waitpid(pid, NULL, 0);
        if(ret < 0)
        {
            perror("waitpid");
            return 404;
        }
        close(input[1]);
        close(output[0]);
    }
    return 200;
}

// 请求处理函数
static void* handle_request(void* arg)
{
    int sock = (int)arg;
    char line[MAX_SIZE];
    int status_code = 200; // 状态码

    // 一行行提取请求信息
    int ch_size = get_line(sock, line, sizeof(line));
    if(ch_size <= 0)
    {
        status_code = 404;
        goto end;
    }
    // 到这里已经提取了第一行
    line[ch_size] = '\0';
    // printf("%s", line);
    // GET /a/b/c HTTP/1.0
    // 首先提取出请求方法
    size_t i = 0;
    size_t j = 0;
    char method[MAX_SIZE/4];
    while(i < sizeof(method)-1 && j < sizeof(line) && !isspace(line[j]))
    {
        method[i++] = line[j++];
    }
    method[i] = '\0';
    // 到这里就已经提取到了请求方法
    // 为了避免请求方法之后不止一个空格, 需要把后面的空格都跳过
    while(j < sizeof(line) && isspace(line[j]))
    {
        j++;
    }
    // 此时 j 指向了 URL
    char url[MAX_SIZE];
    i = 0;
    while(i < sizeof(url)-1 && j < sizeof(line) && !isspace(line[j]))
    {
        url[i] = line[j];
        i++, j++;
    }
    url[i] = '\0';
    urldecode(url);
    // printf("url: %s\n", url);

    // 到这里 url 也提取到了
    // 版本号暂时忽略不提取, 默认 HTTP/1.0
    // 接下来判断请求方法, 因为本服务器仅支持 get 和 post
    // int strcasecmp(const char *s1, const char *s2); 忽略大小写的字符串比较
    if(strcasecmp(method, "GET") != 0 && strcasecmp(method, "POST") != 0)
    {
        // 因为我们的服务器暂时只提供 GET 和 POST 请求的处理
        // 如果请求方法不是 get || post, 就返回错误码, 关闭连接.
        status_code = 404;
        goto end;
    }
    // 到这里肯定是 GET 或 POST
    // CGI: 
    //     1, GET方法 带参数 是CGI, 参数在地址栏
    //     2, POST方法都是CGI, 参数在消息正文, 敏感信息的参数传递都用POST方法
    int cgi_flag = 0;
    char* query_string = NULL;
    if(strcasecmp(method, "POST") == 0)
    {
        cgi_flag = 1;
    }
    else
    {
        // GET 方法
        query_string = url;
        while(*query_string != '\0')
        {
            // https://www.baidu.com/s?ie=utf-8wd=花
            // 我们发现GET方法的URL中只要有问号, 说明就是带参数的, ? 后面的就是参数
            if(*query_string == '?')
            {
                // 把问号前后分离
                *query_string = '\0';
                query_string++;
                cgi_flag = 1;
                break;
            }
            query_string++; // 遍历url
        }
    }
    char resource_path[MAX_SIZE];
    sprintf(resource_path, "webroot%s", url);
    if(resource_path[strlen(resource_path)-1] == '/')
    {
        strcat(resource_path, "index.html");
    }
    // printf("请求方法: %s\n请求路径: %s\n请求参数: %s\n", 
    //        method, url, resource_path);

    // int stat(const char *file_name, struct stat *buf);
    // 通过文件名filename获取文件信息，并保存在buf所指的结构体stat中
    // 执行成功则返回0，失败返回-1，错误代码存于errno
    struct stat st;
    if(stat(resource_path, &st) < 0)
    {
        // 文件不存在
        status_code = 404;
        goto end;
    }
    else
    {
        if(S_ISDIR(st.st_mode)) // 如果请求资源是一个目录
        {
            strcat(resource_path, "index.html"); // 把目录的首页拼接到资源路径中
        }
        // 如果请求的资源具有可执行权限
        else if(
                    (st.st_mode & S_IXUSR) ||
                    (st.st_mode & S_IXGRP) ||
                    (st.st_mode & S_IXOTH)
               )
        {
            cgi_flag = 1;
        }

        if(cgi_flag == 1)
        {
            status_code = exe_cgi(sock, method, resource_path, query_string);
        }
        else
        {
            // 将报头处理完
            handle_hander(sock);
            status_code = echo_www(sock, resource_path, st.st_size);
        }
    }

end:
    if(status_code != 200)
    {
        // 返回状态码信息
        status_response(sock, status_code);
    }
    close(sock);
    return NULL;
}

int startUp(int port)
{
    if(port < 1024)
    {
        printf("port must > 1024 !\n");
        return 2;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0)
    {
        perror("socket");
        return 3;
    }

    // 解决TimeWait引起的bind失败问题
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // 防御空连接
    int keepAlive = 3;// 判定断开前的 KeepAlive 探测次数
    if(setsockopt(listen_fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive,sizeof(keepAlive)) == -1)
    {
        printf("setsockopt SO_KEEPALIVE error!\n");
    }

    int bind_ret = bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    if(bind_ret < 0)
    {
        perror("bind");
        return 4;
    }

    int listen_ret = listen(listen_fd, 5);
    if(listen_ret < 0)
    {
        perror("listen");
        return 5;
    }
    return listen_fd;
}

/////////////////////////////////////////////////////////////////
// 主函数
/////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("Usage : %s [port > 1024]\n", argv[0]);
        return 1;
    }

    // 1, 获取监听套接字
    int listen_sock = startUp(atoi(argv[1]));

    // 忽略 SIGPIPE 信号
    signal(SIGPIPE, SIG_IGN);

    // 2, 循环, 获得新连接
    while(1)
    {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int connect_fd = accept(listen_sock, (struct sockaddr*)&client, &len);
        if(connect_fd < 0)
        {
            perror("accept");
            continue;
        }
        // 到这里说明有新连接来了
#ifdef DEBUG
        char buf_ip[MAX_SIZE/4];
        //清空buf_ip
        buf_ip[0] = 0;
        //inet_ntop转换client_sock的ip
        if(inet_ntop(AF_INET, &client.sin_addr, buf_ip, sizeof(buf_ip)) == NULL)
        {
            perror("inet_ntop");
            return 4;
        }
        printf("get a new connect : [%s | %d]\n", buf_ip, ntohs(client.sin_port));
#endif
        // 创建线程
        // int pthread_create(pthread_t *tidp,const pthread_attr_t *attr,
        //                   (void*)(*start_rtn)(void*),void *arg);
        pthread_t tid = 0;

        int pthread_create_ret = pthread_create(&tid, NULL, handle_request, (void*)connect_fd);

        if(pthread_create_ret < 0)
        {
            perror("pthread_create");
            close(listen_sock);
            return 6;
        }
        // 线程分离
        pthread_detach(tid);
    }
    close(listen_sock);
    return 0;
}

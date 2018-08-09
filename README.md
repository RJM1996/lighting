# Remote lighting control

## 项目介绍

### 智慧家庭

#### 主要技术:
     C/C++, socket 编程, MySQL, HTTP 协议, TCP 协议, 多线程, HTML, Android, NodeMcu, lua, shell

#### 主要功能:
     利用 Android 手机 App 远程控制家用插座, 从而控制家用电器

#### 项目特点:
     1. 开发 Android App，实现简单的登录以及开关控制功能 
     2. 利用 socket 套接字编程实现支持 GET 或 POST 请求的多线程 HTTP 服务器
     3. 利用 CGI 程序连接 MySQL 数据库，处理 App 的注册登录请求 
     4. 利用 CGI 程序将 App 的开关控制命令以及设备 id 转发给 TCP 服务器
     5. 利用 socket 套接字编程实现可以与 NodeMcu 进行长连接的多线程 TCP 服务器
     6. 利用顺序表对来自 NodeMcu 连接进行维护(设备 id 以及连接套接字), 
        通过 HTTP 发来的设备 id 在顺序表查找到对应的连接套接字，并将命令转发给 NodeMcu. 
     7. NodeMcu 解析命令控制 I/O 口输出，通过继电器控制插座


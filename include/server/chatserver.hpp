#ifndef CHATSERVER_H // 头文件保护（Header Guard）或预处理条件编译指令的示例，用于防止头文件被重复包含（或说“多重包含”）在同一个编译单元中
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
using namespace std;
using namespace muduo;
using namespace muduo::net;


//聊天服务器的主类
class ChatServer
{

public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop,
                 const InetAddress &listenAddr,
                 const string &nameArg);

    // 启动服务
    void start();

private:
    // 上报连接相关信息的回调函数
    void onConnection(const TcpConnectionPtr &);

    // 上报读写事件相关信息的回调函数
    void onMessage(const TcpConnectionPtr &,
                   Buffer *,
                   Timestamp);

    TcpServer _server;//组合的muduo库，实现服务器功能的类对象
    EventLoop *_loop;//指向事件循环对象的指针
};

#endif
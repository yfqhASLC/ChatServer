#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>
#include <iostream>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

/*
epoll+线程池
好处：能够把网络I/O的代码和业务区分开
*/

// 基于muduo网络库开发网络服务器程序
/*
1.组成TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer_c的构造函数
4.在当前服务器类的构造函数中，注册处理连接的回调函数和处理读写事件的回调函数
5.注册合适的服务端线程数量，muduo库会自行分配I/O线程和worker线程

*/
class ChatServer_c
{
public:
    ChatServer_c(EventLoop *loop,               // 事件循环
                 const InetAddress &listenAddr, // IP+Port
                 const string &nameArg)     // 服务器名字
    :_server(loop,listenAddr,nameArg),_loop(loop)
    {

        // 给服务器注册用户连接的创建和断开的回调
        _server.setConnectionCallback(std::bind(&ChatServer_c::onConnection, this, _1));

        // 给服务器注册用户读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer_c::onMessage, this, _1, _2, _3));

        // 设置服务器端的线程数量    这里是一个I/O线程+三个woker(工作)线程
        _server.setThreadNum(4);
    }

    // 开启事件循环
    void start()
    {
        _server.start();
    }

private:
    // 专门处理用户的连接创建和断开
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state: online" << endl;
        }
        else
        {
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state: offline" << endl;
            conn->shutdown(); // close(fd)
        }
    }

    // 专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn, // 连接
                   Buffer *buffer,               // 缓冲区
                   Timestamp time)               // 接收到数据的时间信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data: " << buf << " time: " << time.toString() << endl;
        conn->send(buf);
    }

    TcpServer _server; // 1.
    EventLoop *_loop;  // 2.
};

int main()
{

    EventLoop loop;
    InetAddress addr("192.168.84.128", 6000);
    ChatServer_c server(&loop, addr, "ChatServer_c");

    server.start();
    loop.loop(); // epoll_wait以阻塞方式等待新用户连接，已连接用户的读写事件
    return 0;
}

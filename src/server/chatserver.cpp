#include "chatserver.hpp"
#include "json.hpp"
#include"chatservice.hpp"


#include <string>
#include <functional>
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{

    // 注册链接回调
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

    // 注册消息回调
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}

// 启动服务
void ChatServer::start()
{
    _server.start();
}

// 上报连接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{

    // 用户客户端断开连接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer, // 缓冲区
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();

    // 数据的反序列化
    json js = json::parse(buf);

    // 完全解耦网络模块和业务模块的代码
    // 通过js["msgid"]  获取业务handler（处理器），
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());//js["msgid"].get<int>() 将msgid对应的元素强制转为int型
    //回调msgid绑定的事件处理器，来执行相应的业务处理
    msgHandler(conn,js,time);

    
}
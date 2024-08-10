#include "chatservice.hpp"
#include "public.hpp"

#include <muduo/base/Logging.h>
#include <vector>
#include <map>
using namespace std;
using namespace muduo;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 建立消息，以及对应的回调操作
ChatService::ChatService()
{
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGOFF_MSG, std::bind(&ChatService::logoff, this, _1, _2, _3)});

    // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (_redis.connect())
    {
        // 设置上报消息的回报
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 服务器异常，业务重置方法
void ChatService::reset()
{
    // 把online状态的用户，全部设置为offline
    _userModel.resetState();
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志：msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // LOG_ERROR << "msgid:" << msgid << " can not find handler!   这个信息没有相对应的处理器！";

        // 使用lamda表达式返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << "这个信息没有相对应的处理器！== can not find handler! "; // 不用加endl
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 处理登录业务  id    password
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];
    string pwd = js["password"];

    User user = _userModel.query(id);
    if (user.getId() == id && user.getPassword() == pwd)
    {
        if (user.getState() == "online")
        {
            // 账号已经在线，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 1; // 若errno值为0,则成功.
            response["errmsg"] = "该账号已登录(online)，请重新输入账号! == The account is already logged in (online), please re-enter the account!";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex); // 加互斥锁，保证多线程安全     出中括号自动析构解锁
                _userConnMap.insert({id, conn});
            }

            // id用户登陆成功后，向redis订阅channel(id)
            _redis.subscribe(id);

            // 登录成功，更新当前用户的状态信息
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; // 若errno值为0,则成功.
            response["errmsg"] = "登陆成功! == Login successful!";
            response["name"] = user.getName();
            response["id"] = user.getId();

            // 查询该用户是否有离线消息，有则转发
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取用户的离线消息后，将已读取的所有离线消息删除
                _offlineMsgModel.remove(id);
            }

            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                // 非空，有好友，返回好友信息
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
    }
    else if (user.getId() == id && user.getPassword() != pwd)
    {
        // 用户存在，但密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 2; // 若errno值为0,则成功.
        response["errmsg"] = "密码错误！== Password incorrect!";
        conn->send(response.dump());
    }
    else if (user.getId() == -1)
    {

        // 用户不存在，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 3; // 若errno值为0,则成功.
        response["errmsg"] = "用户名不存在！== The username does not exist!";
        conn->send(response.dump());
    }
}

// 处理注册业务  name   password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; // 若errno值为0,则成功.若不为0，则注册失败
        response["errmsg"] = "注册成功！== registered successfully! ";
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1; // 若errno值为0,则成功.若不为0，则注册失败
        response["errmsg"] = "注册失败！请更换用户名重新注册或联系管理者 == Registration failed! please re register with different name or contact the administrator";
        conn->send(response.dump());
    }
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{

    User user;
    {
        lock_guard<mutex> lock(_connMutex); // 加互斥锁，保证多线程安全     出中括号自动析构解锁

        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 用户异常退出，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId());

    // 更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid在线，消息接受者在线，转发消息
            it->second->send(js.dump());
            return;
        }
    }

    // 在sql数据库中查询toid是否在线，若在线，说明在其他服务器中，则publish消息
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    // toid不在线，存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 处理退出登录业务
void ChatService::logoff(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 用户退出登录，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

// 添加好友业务   id    friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    User user = _userModel.query(friendid);

    if (user.getId() == -1)
    {
        // 用户id不存在，添加失败
        json response;
        response["msgid"] = ADD_FRIEND_MSG_ACK;
        response["errno"] = 1; // 若errno值为0,则成功.若不为0，则注册失败
        response["errmsg"] = "用户id不存在, 添加好友失败, 请检查id是否正确! == User ID does not exist, adding friend failed, please check if the ID is correct!";
        conn->send(response.dump());
    }
    else
    {
        // 用户id存在，存储好友信息
        _friendModel.insert(userid, friendid);
        // 用户id存在，添加成功
        json response;
        response["msgid"] = ADD_FRIEND_MSG_ACK;
        response["errno"] = 0; // 若errno值为0,则成功.若不为0，则注册失败
        response["errmsg"] = "添加好友成功！== Friend added successfully!";
        conn->send(response.dump());
    }
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");

        // 创建群组成功
        json response;
        response["msgid"] = CREATE_GROUP_MSG_ACK;
        response["errno"] = 0; // 若errno值为0,则成功.若不为0，则失败
        response["errmsg"] = "创建群组成功！== Successfully created group!";
        conn->send(response.dump());
    }
    else
    {
        // 创建群组失败
        json response;
        response["msgid"] = CREATE_GROUP_MSG_ACK;
        response["errno"] = 1; // 若errno值为0,则成功.若不为0，则失败
        response["errmsg"] = "创建群组失败！请重试或联系管理者 == Failed to create group! Please try again or contact the administrator ";
        conn->send(response.dump());
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    if (_groupModel.addGroup(userid, groupid, "normal"))
    {
        // 加入群组成功
        json response;
        response["msgid"] = ADD_GROUP_MSG_ACK;
        response["errno"] = 0; // 若errno值为0,则成功.若不为0，则失败
        response["errmsg"] = "加入群组成功！== Successfully joined the group!";
        conn->send(response.dump());
    }
    else
    {
        // 加入群组失败
        json response;
        response["msgid"] = ADD_GROUP_MSG_ACK;
        response["errno"] = 1; // 若errno值为0,则成功.若不为0，则失败
        response["errmsg"] = "加入群组失败！请重试或联系管理者 == Failed to join the group! Please try again or contact the administrator";
        conn->send(response.dump());
    }
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {

        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 在线，直接转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线，若在线，说明在其他服务器中，则publish消息
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                // 离线，存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    //json js = json::parse(msg.c_str());

    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}
/*
ORM框架    Object-Relational Mapping
它用于将对象模型与关系数据库中的表结构进行映射。
这种映射允许开发者使用面向对象的方式来操作数据库，而无需编写大量的SQL语句。
ORM框架大大简化了数据库操作，提高了开发效率，同时也增强了代码的可读性和可维护性。

业务层操作的都是对象    将业务层和数据层分开
*/
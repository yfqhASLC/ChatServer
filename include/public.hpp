#ifndef PUBLIC_H
#define PUBLIC_H

/*
server和client的公共头文件
*/

enum EnMsgTyp // enum  枚举
{
    REG_MSG = 0,        // 注册消息 0   如果枚举中的某个成员没有显式地赋予一个值，那么它的值将是前一个成员的值加1（如果这是第一个成员，且没有指定起始值，则默认为0）。
    REG_MSG_ACK,        // 注册响应消息
    LOGIN_MSG,          // 登录消息2
    LOGIN_MSG_ACK,      // 登录响应消息
    ONE_CHAT_MSG,       // 聊天消息 4
    ADD_FRIEND_MSG,     // 添加好友消息5
    ADD_FRIEND_MSG_ACK, // 添加好友响应消息

    CREATE_GROUP_MSG,     // 创建群组消息 7
    CREATE_GROUP_MSG_ACK, // 创建群组响应消息
    ADD_GROUP_MSG,        // 加入群组消息9
    ADD_GROUP_MSG_ACK,    // 加入群组响应消息
    GROUP_CHAT_MSG,       // 群聊天消息11

    LOGOFF_MSG, // 退出登录消息12   LOGINOUT_MSG
    //DELETEID_MSG//注销账号

};

#endif
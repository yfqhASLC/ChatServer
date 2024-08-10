#include "usermodel.hpp"
#include "db.h"

// 表的增加方法   将注册用户加入表中
bool UserModel::insert(User &user)
{

    // 1、组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str()); // c_str   将string转为char*

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 根据用户id查询用户信息
User UserModel::query(int id)
{
    // 1、组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id); //

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 查询成功
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);

                mysql_free_result(res); // 释放指针空间

                return user;
            }
        }
    }
    return User(); // 没有查询到，直接返回User的构造，其中id默认为-1，可以根据id=-1判断没有找到
}

// 更新用户的状态信息
bool UserModel::updateState(User user)
{
    // 1、组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId()); // c_str   将string转为char*

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

// 重置用户的状态信息(服务器故障，导致实际下线，而状仍为online，需要将其重置为offline)
void UserModel::resetState()
{
    // 1、组装sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
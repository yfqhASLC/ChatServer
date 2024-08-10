#ifndef USER_H
#define USER_H

#include <iostream>
using namespace std;

//User表的ORM类
/*
ORM（Object-Relational Mapping）框架是一种在编程中广泛使用的技术，
它用于将对象模型与关系数据库中的表结构进行映射。
*/
class User
{
public:
    User(int Id = -1, string Name = "", string Password = "", string State = "offline")
    {
        this->id = Id;
        this->name = Name;
        this->password = Password;
        this->state = State;
    }

    // 设置user参数
    void setId(int Id) { this->id = Id; }
    void setName(string Name) { this->name = Name; }
    void setPassword(string Password) { this->password = Password; }
    void setState(string State) { this->state = State; }

    // 获得user参数
    int getId() { return this->id; }
    string getName() { return this->name; }
    string getPassword() { return this->password; }
    string getState() { return this->state; }

private:
    int id;
    string name;
    string password;
    string state;
};

#endif
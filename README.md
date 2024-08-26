# ChatServer
C++集群聊天服务器和客户端源码：基于muduo实现 +json+MySQL+nginx+redis

编译方式：  
一：  
1. cd build   
2. rm -rf *  
3. cmake ..  
4. make
二：  
使用自动编译脚本  ./autobuild.sh  

技术栈：  
Json的序列化和反序列化  
muduo网络库开发  
MySQL数据库编程  
CMake构建编译环境  
nginx的tcp负载均衡器配置  
redis缓存服务器编程实践：基于发布-订阅的服务器中间件reids消息队列编程实践  

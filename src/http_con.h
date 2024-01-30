#pragma once
#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <sys/epoll.h>
#include <sstream>
#include <fstream>
#include <fcntl.h>   
#include <unistd.h>   
#include <stdio.h> 
#include <sys/stat.h>
#include <map>
#include <dirent.h>
#include "mysql_conn_pool.h"
#include "logger.h"

class http_con
{
private:
    int m_epoll_fd;//epoll套接字
    int m_client_fd;//客户端套接字
    int m_LE;// 0为LT模式; 1为ET模式
    mysql_conn_pool* m_mysql_pool;//数据库连接池
    char buff[4096];//用于存储客户端发送的信息
    std::string method;//GET或POST
    std::string path;//路径
    map<string,string> users;
    char usAndpsw[1024];//存储post中的用户和密码
    char user[1024];//用户
    char password[1024];//密码
    Logger* m_logger;//日志对象

public:
    http_con();
    void init(int epoll_fd,int client_fd,mysql_conn_pool* mysql_pool,int LE,Logger* logger);//初始化
    ~http_con();
    void Parse_request_line();//解析请求行
    void sendHead(int stats,const char* descr,const char* type);//发送响应头
    void sendfile(const char* fileName);//发送文件
    void sendDir(const char* dirName);//发送目录
    const char* getFileType(const char*name);//获取文件类型
    bool check_user();//验证user和password
    bool insert_user();//
    bool con_client();//与客户端通信
    void conn_LT();//LT模式通信
    void conn_ET();//ET模式通信
};

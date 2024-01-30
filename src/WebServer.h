#pragma once
#include "ThreadPool.h"
#include "mysql_conn_pool.h"
#include "http_con.h"
#include "logger.h"
#include <string>
#include <cstdio>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAX_EPOLL_EVENT 10000

class WebServer
{
private:
	//数据库相关
	std::string m_url;//数据库地址
	int m_port;//数据库端口
	std::string m_user;//数据库用户名
	std::string m_password;//数据库密码
	std::string m_databasename;//数据库表名
	int m_mysql_num;//数据库连接池数量
	mysql_conn_pool* m_mysql_pool;//数据库连接池

	//线程池相关
	int m_thread_num;//线程池线程数量
	ThreadPool<http_con>* m_pool;//线程池

	//epoll相关
	int m_LE;//0表示epoll的LT模式;1表示epoll的ET模式
	int m_epoll_fd;//epoll套接字
	struct epoll_event ev;//epoll属性
	struct epoll_event evs[MAX_EPOLL_EVENT];//存储发生的事件表

	//日志相关
	bool m_log;//日志开关
	Logger* logger;//日志对象
	int m_logQueue_num;//日志异步队列数量

private:
	int m_server_fd;//服务端套接字
	int m_server_port;//服务器端口
	struct sockaddr_in m_addr;//属性

public:
	WebServer();
	~WebServer();
	void init(std::string url,int port,
		std::string user,std::string password,std::string databasename,
		int LT,int mysql_num,int thread_num,bool log,int log_num);
	void thread_pool(); //线程池初始化
	void mysql_pool();	//数据库连接池初始化
	void accept_client();//与客户端建立新连接
	void event_listen(int server_port);//网络编程基本操作
};


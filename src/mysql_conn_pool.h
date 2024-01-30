#pragma once
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <pthread.h>

using namespace std;

class mysql_conn_pool
{
private:
	mysql_conn_pool(std::string url, int port, std::string user, std::string password, std::string databasename,int maxNum);
	int m_port;//数据库端口
	std::string m_url;//数据库地址
	std::string m_user;//数据库用户名
	std::string m_password;//数据库密码
	std::string m_databasename;//数据库表名
	int m_max_num;//数据库连接池最大连接数
	int m_conn_ing;//已连接数
	int m_free_num;//空闲数
	std::queue<MYSQL*> m_queue;//数据库池
	pthread_mutex_t m_mutex;//互斥锁

public:
	~mysql_conn_pool();
	static mysql_conn_pool* mysql_pool;
	MYSQL* getConn();//获取数据库连接
	bool reConn(MYSQL* conn);//释放连接
	int freeConnNum();//获取当前空闲连接数
	static mysql_conn_pool* getObj(std::string url, int port, std::string user, std::string password, std::string databasename,int maxNum);//获取实例
};


#include "mysql_conn_pool.h"

mysql_conn_pool* mysql_conn_pool::mysql_pool = NULL;

mysql_conn_pool::mysql_conn_pool(std::string url, int port, std::string user, std::string password, std::string databasename,int maxNum)
{
	m_url = url;
	m_port = port;
	m_user = user;
	m_password = password;
	m_databasename = databasename;
	m_max_num = maxNum;
	if (pthread_mutex_init(&m_mutex, NULL) != 0)
	{
		std::cout << "pthread_mutex_init Error" << std::endl;
		exit(1);
	}
	for (int i = 0; i < m_max_num; i++)
	{
		MYSQL* con = NULL;
		con = mysql_init(con);

		if (con == NULL)
		{
			std::cout << "MySQL init failed : " << i <<std::endl;
			exit(1);
		}
		con = mysql_real_connect(con, url.c_str(), user.c_str(), password.c_str(), databasename.c_str(), port, NULL, 0);

		if (con == NULL)
		{
			std::cout << "MySQL connect failed : " << i <<std::endl;
			exit(1);
		}
		m_queue.push(con);
		m_free_num++;
	}
}
mysql_conn_pool::~mysql_conn_pool()
{
	pthread_mutex_lock(&m_mutex);
	while (m_queue.size() > 0)
	{
		MYSQL* conn = m_queue.front();
		m_queue.pop();
		mysql_close(conn);
	}
	m_free_num = 0;
	m_conn_ing = 0;
	m_max_num = 0;
	pthread_mutex_unlock(&m_mutex);
	pthread_mutex_destroy(&m_mutex);
}

MYSQL* mysql_conn_pool::getConn()
{
	MYSQL* conn = NULL;
	if (freeConnNum() == 0)
	{
		return NULL;
	}
	pthread_mutex_lock(&m_mutex);
	conn = m_queue.front();
	m_queue.pop();
	m_conn_ing++;
	m_free_num--;
	pthread_mutex_unlock(&m_mutex);
	return conn;
}

bool mysql_conn_pool::reConn(MYSQL* conn)
{
	if (conn == NULL)
	{
		return false;
	}
	pthread_mutex_lock(&m_mutex);
	m_queue.push(conn);
	m_conn_ing--;
	m_free_num++;
	pthread_mutex_unlock(&m_mutex);

	return true;
}

int mysql_conn_pool::freeConnNum()
{
	return this->m_free_num;
}

mysql_conn_pool* mysql_conn_pool::getObj(std::string url, int port, std::string user, std::string password, std::string databasename,int maxNum)
{
	if(mysql_pool == NULL)
	{
		mysql_pool = new mysql_conn_pool(url,port,user,password,databasename,maxNum);
	}
	return mysql_pool;
}


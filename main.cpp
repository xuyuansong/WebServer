#include <iostream>
#include <unistd.h>
#include "src/WebServer.h"

#include <sstream>

using namespace std;

int main()
{
	//创建服务器对象
	WebServer* server = new WebServer();

	//初始化服务器数据库相关，触发模式（0表示LT（水平触发模式）; 1表示ET（边沿触发模式）），
	//数据库连接池中连接数，线程池线程数，是否开启日志系统，日志任务队列大小
	server->init("127.0.0.1",3306,"root","Mysql1234!","yourdb",0,5,6,false,7);

	//epoll监听启动，服务器端口
	server->event_listen(9007);

	return 0;
}

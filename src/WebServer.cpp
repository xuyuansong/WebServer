#include "WebServer.h"

WebServer::WebServer()
{
	logger = new Logger("log.txt");
	logger->log("start Server");
}

WebServer::~WebServer()
{
	delete m_pool;
	delete m_mysql_pool;
	logger->log("stop Server");
}

void WebServer::init(std::string url, int port, std::string user, std::string password, std::string databasename, int LT, int mysql_num, int thread_num, bool log, int log_num)
{
	m_url = url;
	m_port = port;
	m_user = user;
	m_password = password;
	m_databasename = databasename;
	m_LE = LT;
	m_mysql_num = mysql_num;
	m_thread_num = thread_num;
	m_log = log;
	m_logQueue_num = log_num;
	WebServer::mysql_pool();
	//std::cout << "数据库连接池初始化成功"<< std::endl;
	WebServer::thread_pool();
	//std::cout << "线程池初始化成功"<< std::endl;
}

void WebServer::thread_pool()
{
	m_pool = new ThreadPool<http_con>(m_thread_num);
}

void WebServer::mysql_pool()
{
	m_mysql_pool = mysql_conn_pool::getObj(m_url,m_port,m_user,m_password,m_databasename,m_mysql_num);
}

void WebServer::event_listen(int server_port)
{
	m_server_port = server_port;
	m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_server_fd == -1)
	{
		perror("socket server_fd failed");
		logger->log("socket server_fd failed");
		exit(1);
	}
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_addr.sin_port = htons(m_server_port);

	int ret = bind(m_server_fd, (struct sockaddr*)&m_addr, sizeof(m_addr));
	if (ret == -1)
	{
		perror("bind server_fd failed");
		logger->log("bind server_fd failed");
		exit(1);
	}
	ret = listen(m_server_fd, 5);
	if (ret == -1)
	{
		perror("listen server_fd failed");
		logger->log("listen server_fd failed");
		exit(1);
	}

	m_epoll_fd = epoll_create(5);
	if(m_epoll_fd == -1)
	{
		perror("epoll_create epoll_fd failed");
		logger->log("epoll_create epoll_fd failed");
		exit(1);
	}
	ev.data.fd = m_server_fd;
	//通过m_LE判断epoll启用哪种模式: 0表示LT（水平触发模式）; 1表示ET（边沿触发模式）
	if(m_LE == 0)
	{
		ev.events = EPOLLIN;
	}else if(m_LE == 1)
	{
		ev.events = EPOLLIN | EPOLLET ;
	}

	ret = epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,m_server_fd,&ev);
	if(ret == -1)
	{
		perror("epoll_ctl add server_fd failed");
		logger->log("epoll_ctl add server_fd failed");
		exit(1);
	}
	int size = sizeof(evs) / sizeof(struct epoll_event);

	while (true)
	{
		int num = epoll_wait(m_epoll_fd,evs,size,-1);
		for (int i = 0; i < num; i++)
		{
			int curfd = evs[i].data.fd;
			//判断监听到的文件描述符:
			//如果是m_server_fd,表示要与客户端建立新连接
			//否则就是与客户端通信
			if(curfd == m_server_fd)
			{
				//建立新连接
				accept_client();
			}else
			{
				//与客户端通信
				http_con* con = new http_con();
				con->init(m_epoll_fd,curfd,m_mysql_pool,0,logger);
				m_pool->addTask(con);
			}
		}
		
	}
	
}

void WebServer::accept_client()
{
	int client_fd = accept(m_server_fd,NULL,NULL);
	if(client_fd == -1)
	{
		perror("accept failed");
		logger->log("accept failed");
		exit(1);
	}

	int flag = fcntl(client_fd,F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(client_fd,F_SETFL,flag);

	struct epoll_event ev;
	ev.data.fd = client_fd;
	ev.events = EPOLLIN | EPOLLET;

	int ret = epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,client_fd,&ev);
	if(ret == -1)
	{
		perror("epoll_ctl add client_fd failed");
		logger->log("epoll_ctl add client_fd failed");
		exit(1);
	}
}

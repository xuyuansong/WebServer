#include "http_con.h"

http_con::http_con()
{
}

http_con::~http_con()
{
    memset(buff,0,sizeof(buff));
    m_mysql_pool = NULL;
    close(m_client_fd);
}

void http_con::init(int epoll_fd,int client_fd,mysql_conn_pool* mysql_pool,int LE,Logger* logger)
{
    m_epoll_fd = epoll_fd;
    m_client_fd  = client_fd;
    m_mysql_pool = mysql_pool;
    m_LE = LE;
    m_logger=logger;
    memset(buff,0,sizeof(buff));
}

bool http_con::con_client()
{
    chdir("workpath");
    if(m_LE == 0)
    {
        conn_LT();
    }else
    {
        conn_ET();
    }
    char* file = NULL;

    if(method == "GET" && path == "/")
    {
        file = "judge.html";
    }else if(method == "POST" && path == "/welcome.html")
    {
        if(check_user())
        {
            file = "Resource";
            std::string str(user);
            m_logger->log("user : ["+str+"] login succeed");
        }else
        {
            path = "logError.html";
            file = const_cast<char*>(path.c_str());
            std::string str(user);
            m_logger->log("user : ["+str+"] login failed");
        }
    }else if(method == "POST" && path == "/register")
    {
        if(check_user())
        {
            path = "registerError.html";
            file = const_cast<char*>(path.c_str());
        }else
        {
            insert_user();
            //std::cout<< "注册成功"<<user<<password<<std::endl;
            path = "log.html";
            file = const_cast<char*>(path.c_str());
            std::string str(user);
            m_logger->log("register succeed user : "+str);
        }
    }
    else
    {
        file=const_cast<char*>(path.c_str()) + 1;
    }
    struct stat st;
    int ret = stat(file,&st);
    if (ret == -1)
    {
        //文件不存在
        chdir("workpath");
        sendHead(404,"Not Find",getFileType(".html"));
        sendfile("404.html");
        return false;
    }
    
    if(S_ISDIR(st.st_mode))
    {
        //发送目录
        sendHead(200,"OK",getFileType(".html"));
        sendDir(file);
        std::string str(file);
        m_logger->log("user visit directory file : "+str);
    }else
    {
        //发送文件内容
        chdir("workpath");
        sendHead(200,"OK",getFileType(file));
        sendfile(file);
        std::string str(file);
        m_logger->log("user visit file : "+str);
    }
}

void http_con::Parse_request_line()
{ 
    //std::cout << buff <<std::endl;
    std::stringstream ss(buff);
    std::string line;
      
    // 读取第一行，并分割出方法、路径和其他信息  
    std::getline(ss, line);
    size_t pos1 = line.find(' ');  
    size_t pos2 = line.find(' ', pos1+1);
    method = line.substr(0, pos1);  
    path = line.substr(pos1+1, pos2-pos1-1);

    const char* dot = strrchr(buff, '\r\n');
    strcpy(usAndpsw,dot+1);
    if(usAndpsw != "")
    {
        memset(user,0,sizeof(user));
	    memset(password,0,sizeof(password));
        sscanf(usAndpsw,"user=%[^&]&password=%[^ ]",user,password);
    }
}

void http_con::sendHead(int stats, const char *descr, const char *type)
{
    stringstream response;
    response << "HTTP/1.1 "<<stats<<" "<<descr<<"\r\n";
    response << "Content-Type: "<< type <<"\r\n"; // 修改Content-Type为视频类型，如video/mp4
    response << "\r\n";
    
    send(m_client_fd, response.str().c_str(), response.str().size(), 0);
}

void http_con::conn_LT()
{
    int len = recv(m_client_fd,buff,sizeof(buff),0);
    //判断是否接收完毕
    if(len > 0)
    {
        //解析请求行
        Parse_request_line();
    }else if(len == 0)
    {
        //客户端断开连接,因此将m_client_fd从epoll中删除
        epoll_ctl(m_epoll_fd,EPOLL_CTL_DEL,m_client_fd,NULL);
        close(m_client_fd);
    }else{
        perror("recv failed");
        exit(1);
    }
}

void http_con::conn_ET()
{
    int len = 0,tot = 0;
    char temp[1024] = { 0 };
    while ((len=recv(m_client_fd,temp,sizeof(temp),0)) > 0)
    {
        if(len+tot < sizeof(buff))
        {
            memcpy(buff+tot,temp,len);
        }
        tot+=len;
    }
    //判断是否接收完毕
    if(len == -1 && errno == EAGAIN)
    {
        //解析请求行
        Parse_request_line();
    }else if(len == 0)
    {
        //客户端断开连接,因此将m_client_fd从epoll中删除
        epoll_ctl(m_epoll_fd,EPOLL_CTL_DEL,m_client_fd,NULL);
        close(m_client_fd);
    }else{
        perror("recv failed");
        exit(1);
    }
}

const char *http_con::getFileType(const char *name)
{
    const char* dot = strrchr(name, '.'); // 自右向左查找‘.'字符  
    if (dot == NULL) {  
        return "text/plain;charset=utf-8"; // 纯文本  
    }  
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) {  
        return "text/html;charset=utf-8";  
    }  
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) {  
        return "image/jpeg";  
    }  
    if (strcmp(dot, ".gif") == 0) {  
        return "image/gif";  
    }  
    if (strcmp(dot, ".png") == 0) {  
        return "image/png";  
    }  
    if (strcmp(dot, ".css") == 0) {  
        return "text/css";  
    }  
    if (strcmp(dot, ".mp4") == 0) {  
        return "video/mp4";  
    }  
    if (strcmp(dot, ".wav") == 0) {  
        return "audio/wav"; // 注意这里应该是audio/wav，而不是wav。wav是一种音频文件格式。  
    }  
    return "text/plain;charset=utf-8"; // 其他未知类型的文件 
}

bool http_con::check_user()
{
    MYSQL* con = m_mysql_pool->getConn();
    if (mysql_query(con, "SELECT username,passwd FROM user"))
    {
        perror("mysql_query failed");
    }
    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(con);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
    }
    return users[user] == password;
}

bool http_con::insert_user()
{
    MYSQL* con = m_mysql_pool->getConn();
    char insert[1024];
    sprintf(insert,"INSERT INTO user VALUES ('%s', '%s') ",user,password);
    return mysql_query(con,insert);
}


void http_con::sendfile(const char* fileName)
{
    int fd = open(fileName,O_RDONLY);
    if(fd == -1)
    {
        perror("open file failed");
        exit(1);
    }
    while (true)
    {
        char message[4096];
        memset(message,0,sizeof(message));
        int len = read(fd,message,sizeof(message));
        if (len>0)
        {
            send(m_client_fd,message,sizeof(message),0);
            usleep(1);
        }else if(len == 0)
        {
            break;
        }else
        {
            perror("read failed");
        }
    }
}

void http_con::sendDir(const char *dirName)
{
    char buf[4096]={0};
    sprintf(buf,"<html><head><title>%s</title></head><body><table>",dirName);
    struct dirent** namelist;
    int num = scandir(dirName,&namelist,NULL,alphasort);
    for (int i = 0; i < num; i++)
    {
        char* name = namelist[i]->d_name;
        struct stat st;
        char subpath[1024]={0};
        sprintf(subpath,"%s/%s",dirName,name);
        stat(subpath,&st);
        if(S_ISDIR(st.st_mode))
        {
            sprintf(buf+strlen(buf),"<tr><td><a href=\"%s/\">%s</a></td></tr>",name,name);
        }else
        {
            sprintf(buf+strlen(buf),"<tr><td><a href=\"%s\">%s</a></td></tr>",name,name);
        }
        send(m_client_fd,buf,sizeof(buf),0);
        memset(buf,0,sizeof(buf));
        free(namelist[i]);
    }
    sprintf(buf,"</table></body></html>");
    send(m_client_fd,buf,sizeof(buf),0);
    free(namelist);
    
}
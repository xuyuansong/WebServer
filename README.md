Linux下C++并发Web服务器

* 使用 **线程池 + 非阻塞socket + epoll(LT实现)** 的并发模型
* 解析HTTP请求报文，支持解析**GET和POST**请求
* 访问服务器数据库实现web端用户**注册、登录**功能
* 用户可以**查看图片和视频**


介绍
------------
* /src文件下
    * 源代码
* /workpath
    * 工作，资源目录
* log.txt
    * 日志文件
* main.cpp
    * main函数
* makefile文件
* start.sh
    * 运行shart.sh文件即可运行服务器:**sh shart.sh**

快速运行
------------
* 服务器测试环境
	* Centos7版本7.9.2009
	* MySQL版本5.7.29
* 浏览器测试环境
	* Windows、Linux均可
	* Chrome
	* FireFox
	* 其他浏览器暂无测试
* 测试前确认已安装MySQL数据库

    ```C++
    // 建立yourdb库
    create database yourdb;

    // 创建user表
    USE yourdb;
    CREATE TABLE user(
        username char(50) NULL,
        passwd char(50) NULL
    )ENGINE=InnoDB;

    // 添加数据
    INSERT INTO user(username, passwd) VALUES('name', 'password');
    ```
* 修改main.cpp中的初始化信息

    ```C++
    //创建服务器对象
	WebServer* server = new WebServer();

	//初始化服务器数据库相关，触发模式（0表示LT（水平触发模式）; 1表示ET（边沿触发模式）），
	//数据库连接池中连接数，线程池线程数，是否开启日志系统，日志任务队列大小
	server->init("127.0.0.1",3306,"root","Mysql1234!","yourdb",0,5,6,false,7);

	//epoll监听启动，服务器端口
	server->event_listen(9007);
    ```
* 启动web服务
   ```C++
    sh ./start.sh
   ```
   
* 浏览器端

    ```C++
    ip:9007 //main.cpp中监听的服务器端口
    ```

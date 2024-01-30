CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp  src/mysql_conn_pool.cpp src/WebServer.cpp src/http_con.cpp src/ThreadPool.h
	$(CXX) -o server  $^ $(CXXFLAGS) -L/usr/lib64/mysql -lmysqlclient -lpthread

clean:
	rm  -r server

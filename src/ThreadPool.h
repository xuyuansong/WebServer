
#include <pthread.h>
#include <queue>
#include <cstdio>

template <typename T>
class ThreadPool
{
private:
	int m_threadNum;//线程池中的线程数
	pthread_mutex_t m_mutex;//互斥锁
	pthread_cond_t m_cond;//条件变量
	pthread_t* threadId;//线程ID
	std::queue<T *> m_queue;//任务队列

	static void* work(void* arg);//工作线程任务函数

	void run();

public:
	ThreadPool(int threadNum);
	~ThreadPool();
	void addTask(T* task);//添加任务
};

template <typename T>
ThreadPool<T>::ThreadPool(int threadNum)
{
    m_threadNum = threadNum;
    threadId = new pthread_t[threadNum];
    if(!threadId)
    {
        throw std::exception();
    }
    //互斥锁和条件变量初始化
    if(pthread_mutex_init(&m_mutex,NULL) !=0 && pthread_cond_init(&m_cond,NULL) != 0)
    {
        throw std::exception();
    }

    for (int i = 0; i < threadNum; i++)
    {
        //创建线程
        pthread_create(&threadId[i],NULL,work,this);
    }
    
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    for (int i = 0; i < m_threadNum; i++)
	{
		pthread_cond_signal(&m_cond);
	}
	if (threadId) delete[] threadId;
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

template <typename T>
void* ThreadPool<T>::work(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    pool->run();
    return pool;
}

template <typename T>
void ThreadPool<T>::run()
{
	while (1)
    {
        pthread_mutex_lock(&m_mutex);
        while(m_queue.size() == 0)
        {
            pthread_cond_wait(&m_cond,&m_mutex);
        }
        T *task = m_queue.front();
        m_queue.pop();
        pthread_mutex_unlock(&m_mutex);
        task->con_client();
    }
}

template <typename T>
void ThreadPool<T>::addTask(T* task)
{
    m_queue.push(task);
    pthread_cond_signal(&m_cond);
}



















/*
template<typename T>
class ThreadPool
{
private:
	int threadNum;
	static ThreadPool* threadpool;
	ThreadPool(int threadNum);
	//ThreadPool(const ThreadPool& obj) = delete;
	//ThreadPool& operator=(const ThreadPool& obj) = delete;
	std::queue<T *> m_queue;
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
	pthread_t* threadId;
	static void* worker(void* arg);
public:
	~ThreadPool();
	void addTask(T* task);
	ThreadPool* getObj(int num);
};
*/

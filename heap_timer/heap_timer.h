#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <vector>
#include <unordered_map>
#include <functional>

#include <time.h>
#include "../log/log.h"
#include "../http/http_conn.h"


typedef std::function<void(int)> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

//定时器类:以一个heap实现
class TimerNode {
public:
    int sockfd;             // fd
    time_t expire;          // 超时时间
    //回调函数:从内核事件表删除事件，关闭文件描述符，释放连接资源
    TimeoutCallBack cb;
    bool operator<(const TimerNode &t) {
        return expire < t.expire;
    }
};

//定时器容器类
class TimerManager {
public:
    TimerManager() { heap_.reserve(64) };
    ~TimerManager() {};

    //添加定时器，内部调用私有成员add_timer
    void add_timer(int id, int timeout, const TimeoutCallBack& cb);

    ////处理过期的定时器
    void handle_expired_event();

    //下一次处理过期定时器的时间
    int getNextHandle();

    //在 HTTP 连接的处理过程中需要的对某一个连接对应定时器的过期时间做出改变
    void update(int id, int timeout);

    //删除指定id节点，并且用指针触发处理函数
    void work(int id);

    void pop();

    int count(int id);

private:
    std::vector<TimerNode> heap_; //存储定时器
    std::unordered_map<int, int> ref_;  // 映射一个fd对应的定时器在heap中的位置

    void del_(size_t i);//删除指定定时器
    void siftup_(size_t i);//向上调整
    bool siftdown_(size_t index, size_t n);//向下调整
    void swapNode_(size_t i,size_t j);//交换两个结点位置
};

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    //初始化周期
    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;//管道id
    TimerManager m_timer_heap;//小根堆定时器
    static int u_epollfd;//epollfd
    int m_TIMESLOT;//最小时间间隙
};
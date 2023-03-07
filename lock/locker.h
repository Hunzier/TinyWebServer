#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/*
sem有两个主要操作：wait（等待）和signal（发信号）。
wait操作将sem的计数器减一，如果计数器为零，则该线程将被阻塞，直到其他线程执行signal操作将计数器加一。
signal操作将sem的计数器加一，如果有线程因为wait操作被阻塞，则会唤醒一个线程。
在多线程编程中，sem可以用于控制多个线程对共享资源的访问。
例如，可以使用sem实现一个线程池，线程池中的线程在处理任务时需要获取一个计数器为1的sem，表示只有一个线程可以访问共享资源，其他线程必须等待。
当某个线程处理完任务后，会执行signal操作释放sem，让其他线程可以获取sem并继续处理任务。
另外，在Linux系统中，sem可以使用POSIX信号量或System V信号量两种方式进行实现。
*/

class sem //信号量类
{
public:
    sem()   //信号量PV操作
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

//互斥量是一种线程同步机制，用于防止多个线程同时访问共享资源，从而避免竞态条件的发生。
class locker
{
public:
    locker()
    {
        //初始化互斥量mutex
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    pthread_mutex_t *get()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};

//条件变量是利用线程间共享的全局变量进行同步的一种机制，主要包括两个动作：一个线程等待"条件变量的条件成立"而挂起；
//另一个线程使"条件成立"（给出条件成立信号）。为了防止竞争，条件变量的使用总是和一个互斥锁结合在一起。
class cond
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }

    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#endif

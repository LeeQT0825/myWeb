#ifndef __MYWEB_MTHREAD_H__
#define __MYWEB_MTHREAD_H__

#include <pthread.h>
#include <functional>
#include <memory>
#include <string>
#include "mmutex.h"

namespace myWeb{

class Thread{
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb,const std::string& name);       // 创建线程
    ~Thread();

    // 获取内核线程ID
    pid_t getID() const {
        return m_id;
    }
    // 获取线程名称
    const std::string& getName() const {
        return m_name;
    }
    // 等待线程完成
    void join();
    // pthread_t get_pthreadID(){
    //     return m_thread;
    // }

    // 获取当前线程的指针
    static Thread* getThisThread();
    // 获取当前线程名称
    static const std::string& getThisThreadName();
    // 设置当前线程的名称
    static void setThisThreadName(const std::string& name);
   
private:
    Thread(const Thread&)=delete;       // Thread类不可被拷贝
    Thread(const Thread&&)=delete;
    Thread& operator=(const Thread&)=delete;  

    // 执行入口
    static void* Th_Create(void* args);
private:

    pid_t m_id=-1;               // 内核线程号
    pthread_t m_thread=0;        // 用户线程号，由pthread_create()创建
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore m_sem;
};

}

#endif
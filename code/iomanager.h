#ifndef __MYWEB_IOMANAGER_H__
#define __MYWEB_IOMANAGER_H__

#include "scheduler.h"
#include <sys/epoll.h>

namespace myWeb{

// IO协程调度器
class IOManager: public Scheduler{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWmutex lock_type;
    
    // 事件
    enum Event{
        // 无事件
        NONE=0x0,
        // 读事件
        READ=0x1,
        // 无事件
        WRITE=0x2,
    };

    IOManager(size_t num_of_thread=1,bool use_caller=true,const std::string& name=" ");
    ~IOManager();

    // 添加并注册事件(描述符fd发生了event事件，则回调cb，ret=0——成功，ret=-1——失败)
    int addEvent(int fd,Event event,std::function<void()> cb=nullptr);
    // 删除事件
    bool delEvent(int fd,Event event);
    // 取消事件
    bool cancelEvent(int fd,Event event);
    // 取消所有事件
    bool cancelAllEvent(int fd);

    // 返回当前 IOManager
    static IOManager* getThis();

protected:
    // 通知
    void tickle() override;
    // 判断结束状态
    bool stopping() override;
    // 空闲线程执行函数
    void idle() override;
    // 重置事件池大小
    void fdContextResize(size_t size);

private:
    // 句柄分发模块（IO事件调度：支持为描述符注册可读或可写事件的回调函数）
    struct FdContext
    {
        typedef MutexLock fd_lock_type;
        // 事件回调类
        struct EventContext
        {
            Scheduler::ptr e_scheduler;     // 事件执行的调度器
            Fiber::ptr e_fiber;             // 事件协程
            std::function<void()> e_func;   // 事件回调函数
        };
        
        int fd=0;                     // 事件句柄   
        Event fd_event=NONE;          // 当前事件状态 
        EventContext read_event;    // 读事件调度        
        EventContext write_event;   // 写事件调度
        fd_lock_type f_lock;
    };

private:
    // 锁
    lock_type m_lock;
    // epoll事件表句柄
    int m_epfd;
    // 通知管道
    int m_ticklefd[2];
    // 等待执行的事件数量
    std::atomic<size_t> m_pendingEventCount{0};
    // 句柄事件池
    std::vector<FdContext*> m_fdcontexts;
};

}

#endif
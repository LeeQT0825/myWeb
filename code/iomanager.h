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
        READ=0x001,
        // 无事件
        WRITE=0x004,
    };

    IOManager(size_t num_of_thread=1,bool use_caller=true,const std::string& name=" ");
    ~IOManager();

    // 给句柄添加并注册事件(描述符fd发生了event事件，则回调cb，ret=0——成功，ret=-1——失败)
    int addEvent(int fd,Event event,std::function<void()> cb=nullptr);
    // 给句柄删除事件（不会触发执行）
    bool delEvent(int fd,Event event);
    // 给句柄取消事件（找到对应事件，并强制触发执行）
    bool cancelEvent(int fd,Event event);
    // 给句柄取消所有事件
    bool cancelAllEvent(int fd);

    // 返回当前 IOManager
    static IOManager* getThis();

protected:
    // 通知线程，使线程从 idle() 中 yield 出来
    void tickle() override;
    // 判断结束状态
    bool isStopping() override;
    // 空闲线程执行函数（阻塞在 epoll_wait 上等待 tickle() 或其他就绪事件）
    void idle() override;
    // 重置事件池大小
    void fdContextResize(size_t size);

private:
    // 句柄分发模块（IO事件调度：支持为描述符注册可读或可写事件的回调函数）
    struct FdContext
    {
        typedef MutexLock fd_lock_type;
        // 事件回调类
        struct EventCallBack
        {
            Scheduler* e_scheduler=nullptr;     // 事件执行的调度器
            Fiber::ptr e_fiber;             // 事件协程
            std::function<void()> e_func;   // 事件回调函数
        };
        // 获取某一事件的回调
        EventCallBack& getEventCB(Event event);
        // 重置某一事件的回调
        void resetEventCB(EventCallBack& event_cb);
        // 根据事件触发回调
        void triggerEvent(Event event);
        
        int fd=0;                     // 事件句柄   
        Event fd_event=NONE;          // 当前事件状态 
        EventCallBack read_cb;    // 读事件调度        
        EventCallBack write_cb;   // 写事件调度
        fd_lock_type f_lock;
    };

private:
    // 锁
    lock_type m_lock;
    // epoll事件表句柄
    int m_epfd;
    // 通知管道(tickle() 和 idle() 之间的通知)
    int m_ticklefd[2];
    // 等待执行的事件数量
    std::atomic<size_t> m_pendingEventCount{0};
    // 句柄事件池
    std::vector<FdContext*> m_fdcontexts;
};

}

#endif
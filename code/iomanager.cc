#include "iomanager.h"
#include "macro.h"
#include "log.h"
#include "fiber.h"
#include <sys/eventfd.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define MAX_EVENTSIZE 256
#define MAX_TIMEOUT 3000

namespace myWeb{

IOManager* IOManager::getThis(){
    return dynamic_cast<IOManager*>(Scheduler::getScheduler());
}

IOManager::IOManager(size_t num_of_thread,bool use_caller,const std::string& name)
        :Scheduler(num_of_thread,use_caller,name){
    m_epfd=epoll_create(5000);
    MYWEB_ASSERT(m_epfd>0);
    int ret=pipe(m_ticklefd);
    MYWEB_ASSERT(!ret);

    epoll_event epEvent;
    memset(&epEvent,0,sizeof(epEvent));
    epEvent.events= EPOLLIN | EPOLLET;
    epEvent.data.fd=m_ticklefd[0];      // 读端口

    ret=fcntl(m_ticklefd[0],F_SETFL,O_NONBLOCK);
    MYWEB_ASSERT(!ret);

    ret=epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_ticklefd[0],&epEvent);
    MYWEB_ASSERT(!ret);

    fdContextResize(32);
    start();    // Scheduler启动
}

IOManager::~IOManager(){
    stop();
    close(m_epfd);
    SpinLock lock;
    {
        SpinLock::scoped_lock lck(lock);
        close(m_ticklefd[0]);
        close(m_ticklefd[1]);
        lck.unLock();
    }

    for(auto& i:m_fdcontexts){
        if(i){
            delete i;
        }
    }
}

int IOManager::addEvent(int fd,Event event,std::function<void()> cb){
    FdContext* fd_ctx=nullptr;
    if(fd>=m_fdcontexts.size()){
        lock_type::write_lock wr_lck(m_lock);
        if(fd>m_fdcontexts.size()){
            fdContextResize(fd*1.5);
        }
    }
    lock_type::read_lock rd_lck(m_lock);
    fd_ctx=m_fdcontexts[fd];
    rd_lck.unLock();

    FdContext::fd_lock_type::scoped_lock fd_lck(fd_ctx->f_lock);
    // 如果已存在该事件
    if(fd_ctx->fd_event & event){     // 位运算
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"addEvent assert! fd: "<<fd
                                            <<" event: "<<(EPOLL_EVENTS)fd_ctx->fd_event;
        MYWEB_ASSERT(!(fd_ctx->fd_event & event));
    }
    
    // 注册事件
    int op=fd_ctx->fd_event?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
    epoll_event ep_event;
    memset(&ep_event,0,sizeof(ep_event));
    ep_event.data.ptr=fd_ctx;
    ep_event.events=fd_ctx->fd_event | EPOLLET | event;      // TODO 后续可以检测ET和LT模式的差别
    int ret=epoll_ctl(m_epfd,op,fd,&ep_event);
    if(ret==-1){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"epoll_ctl("<<m_epfd<<","
                            <<op<<","<<fd<<","<<(EPOLL_EVENTS)ep_event.events<<"): "
                            <<"("<<errno<<")-("<<strerror(errno)<<")";
        return -1;
    }
    ++m_pendingEventCount;

    // 添加回调（完善 FdContext 类）
    fd_ctx->fd_event = (Event)(fd_ctx->fd_event | event);
    FdContext::EventCallBack& event_cb=fd_ctx->getEventCB(event);
    MYWEB_ASSERT(!event_cb.e_scheduler &&
                    !event_cb.e_fiber &&
                    !event_cb.e_func);
    event_cb.e_scheduler=Scheduler::getScheduler();
    if(cb){
        event_cb.e_func.swap(cb);
    }else{
        event_cb.e_fiber=Fiber::getThis();      // 回到当前协程
        MYWEB_ASSERT(event_cb.e_fiber->getState()==Fiber::State::EXEC);
    }

    return 0;
}

bool IOManager::delEvent(int fd,Event event){
    lock_type::read_lock rd_lck(m_lock);
    if(fd>=m_fdcontexts.size()){
        return false;
    }
    FdContext* fd_ctx=m_fdcontexts[fd];
    rd_lck.unLock();

    FdContext::fd_lock_type::scoped_lock fd_lck(fd_ctx->f_lock);
    if(!(fd_ctx->fd_event & event)){
        return false;
    }

    // 修改或删除注册事件
    Event new_event=(Event)(fd_ctx->fd_event & (~event));
    int op=new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event ep_event;
    ep_event.events= EPOLLET | new_event;
    ep_event.data.ptr=fd_ctx;
    int ret=epoll_ctl(m_epfd,op,fd,&ep_event);
    if(ret==-1){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"epoll_ctl("<<m_epfd<<","
                            <<op<<","<<fd<<","<<(EPOLL_EVENTS)ep_event.events<<"): "
                            <<"("<<errno<<")-("<<strerror(errno)<<")";
        return false;
    }
    --m_pendingEventCount;

    // 修改回调（完善FdContext类）
    fd_ctx->fd_event= new_event;
    FdContext::EventCallBack& event_cb=fd_ctx->getEventCB(event);
    fd_ctx->resetEventCB(event_cb);
    
    return true;
}

bool IOManager::cancelEvent(int fd,Event event){
    lock_type::read_lock rd_lck(m_lock);
    if(fd>=m_fdcontexts.size()){
        return false;
    }
    FdContext* fd_ctx=m_fdcontexts[fd];
    rd_lck.unLock();
    
    FdContext::fd_lock_type::scoped_lock fd_lck(fd_ctx->f_lock);
    if(!(fd_ctx->fd_event & event)){
        return false;
    }

    // 修改或删除注册事件
    Event new_event=(Event)(fd_ctx->fd_event & (~event));
    int op=new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event ep_event;
    ep_event.events= EPOLLET | new_event;
    ep_event.data.ptr=fd_ctx;
    int ret=epoll_ctl(m_epfd,op,fd,&ep_event);
    if(ret==-1){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"epoll_ctl("<<m_epfd<<","
                            <<op<<","<<fd<<","<<(EPOLL_EVENTS)ep_event.events<<"): "
                            <<"("<<errno<<")-("<<strerror(errno)<<")";
        return false;
    }
    --m_pendingEventCount;

    // 触发回调（完善FdContext类）
    fd_ctx->triggerEvent(event);
    
    return true;
}

bool IOManager::cancelAllEvent(int fd){
    lock_type::read_lock rd_lck(m_lock);
    if(fd>=m_fdcontexts.size()){
        return false;
    }
    FdContext* fd_ctx=m_fdcontexts[fd];
    rd_lck.unLock();
    
    FdContext::fd_lock_type::scoped_lock fd_lck(fd_ctx->f_lock);
    if(!fd_ctx->fd_event){
        return false;
    }

    // 删除注册事件
    int op=EPOLL_CTL_DEL;
    epoll_event ep_event;
    ep_event.events=0;
    ep_event.data.ptr=fd_ctx;
    int ret=epoll_ctl(m_epfd,op,fd,&ep_event);
    if(ret==-1){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"epoll_ctl("<<m_epfd<<","
                            <<op<<","<<fd<<","<<(EPOLL_EVENTS)ep_event.events<<"): "
                            <<"("<<errno<<")-("<<strerror(errno)<<")";
        return false;
    }

    // 触发回调（完善FdContext类）
    if(fd_ctx->fd_event & READ){
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->fd_event & WRITE){
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }
    
    MYWEB_ASSERT(fd_ctx->fd_event==0);
    return true;
}

void IOManager::tickle(){
    if(!hasIdleThread()){
        return;
    }
    int ret=write(m_ticklefd[1],"T",1);
    MYWEB_ASSERT(ret==1);
}

bool IOManager::isStopping(){
    return m_pendingEventCount==0 && Scheduler::isStopping();
}

void IOManager::idle(){
    epoll_event* ep_events=new epoll_event[MAX_EVENTSIZE]();
    std::shared_ptr<epoll_event> sh_epEvent(ep_events,[](epoll_event* ptr){delete[] ptr;});      // 用智能指针接管裸指针
    
    while(!isStopping()){
        int ret=0;
        while(true){
            ret=epoll_wait(m_epfd,sh_epEvent.get(),MAX_EVENTSIZE,MAX_TIMEOUT);   //阻塞在 epoll_wait ,毫秒级
            if(ret>0){
                break;
            }
        }
        // 遍历处理就绪事件
        for(int i=0;i<ret;++i){
            epoll_event& ready_event=ep_events[i];
            if(ready_event.data.fd==m_ticklefd[0]){
                uint8_t dump;
                while(read(m_ticklefd[0],&dump,sizeof(dump))>0){}
                continue;
            }

            FdContext* fd_ctx=(FdContext*)ready_event.data.ptr;
            FdContext::fd_lock_type::scoped_lock f_lck(fd_ctx->f_lock);
            // 如果就绪事件为“错误”或“中断”，则同时触发读事件和写事件
            if(ready_event.events & (EPOLLERR | EPOLLHUP)){
                ready_event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->fd_event;
            }
            // 转换为自己定义 Event
            int event=NONE;     // 细节，类型要设为支持 | 的类型
            if(ready_event.events & EPOLLIN){
                event |= READ ;
            }
            if(ready_event.events & EPOLLOUT){
                event |= WRITE;
            }

            // 修改ready_event，再注册到 epoll_wait 里
            int left_event=fd_ctx->fd_event & (~event);
            int op=left_event? EPOLL_CTL_MOD : EPOLL_CTL_DEL ;
            ready_event.events=left_event | EPOLLET;
            int ret=epoll_ctl(m_epfd,op,fd_ctx->fd,&ready_event);
            if(ret==-1){
                INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"epoll_ctl("<<m_epfd<<","
                                    <<op<<","<<fd_ctx->fd<<","<<(EPOLL_EVENTS)ready_event.events<<"): "
                                    <<"("<<errno<<")-("<<strerror(errno)<<")";
                continue;
            }

            // 执行事件回调
            if(event & READ){
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if(event & WRITE){
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }      
        }

        Fiber::yieldToHold();
    }
}

void IOManager::fdContextResize(size_t size){
    m_fdcontexts.resize(size);

    for(size_t i=0;i<size;++i){
        if(!m_fdcontexts[i]){
            m_fdcontexts[i]=new FdContext();
            m_fdcontexts[i]->fd=i;
        }
    }
}

IOManager::FdContext::EventCallBack& IOManager::FdContext::getEventCB(Event event){
    switch(event){
        case Event::READ:
            return read_cb;
        case Event::WRITE:
            return write_cb;
        default:
            MYWEB_ASSERT_2(false,"getEventCB");
    }
    throw std::invalid_argument("getEventCB error");
}

void IOManager::FdContext::resetEventCB(IOManager::FdContext::EventCallBack& event_cb){
    event_cb.e_fiber.reset();
    event_cb.e_func=nullptr;
    event_cb.e_scheduler=nullptr;
}

void IOManager::FdContext::triggerEvent(Event event){
    // 检查事件是否为注册事件，如果是，则注销掉事件
    MYWEB_ASSERT(fd_event & event);
    fd_event = (Event)(fd_event & (~event));

    // 执行回调
    // 细节：执行trigger后要将回调删除，可以通过传指针的方式，类型会直接传到 FIberAndThread 里调用 swap ，
    // 这样 FdContext::EventCallBack 中的回调就会置位 nullptr 。
    EventCallBack& event_cb=getEventCB(event);
    if(event_cb.e_fiber){
        event_cb.e_scheduler->schedule(&event_cb.e_fiber);      
    }else{
        event_cb.e_scheduler->schedule(&event_cb.e_func);
    }
    resetEventCB(event_cb);
    
    return;
}

}
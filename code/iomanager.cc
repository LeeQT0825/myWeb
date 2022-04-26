#include "iomanager.h"
#include "macro.h"
#include "log.h"
#include <sys/eventfd.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

namespace myWeb{

IOManager* IOManager::getThis(){

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
    if(fd>=m_fdcontexts.size()){
        lock_type::write_lock wr_lck(m_lock);
        if(fd>m_fdcontexts.size()){
            fdContextResize(fd*1.5);
        }
    }
    // 如果已存在该事件
    if(m_fdcontexts[fd] && (m_fdcontexts[fd]->fd_event & event)){     // 位运算
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"addEvent assert! fd: "<<fd
                                            <<" event: "<<(EPOLL_EVENTS)m_fdcontexts[fd]->fd_event;
        MYWEB_ASSERT(!(m_fdcontexts[fd]->fd_event & event));
    }
    
    // 注册事件
    int op=m_fdcontexts[fd]->fd_event?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
    epoll_event epEvent;
    memset(&epEvent,0,sizeof(epEvent));
    epEvent.data.ptr=m_fdcontexts[fd];
    epEvent.events=m_fdcontexts[fd]->fd_event | EPOLLET | event;
    int ret=epoll_ctl(m_epfd,op,fd,&epEvent);
    if(ret==-1){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"epoll_ctl("<<m_epfd<<","
                            <<op<<","<<fd<<","<<(EPOLL_EVENTS)epEvent.events<<"): "
                            <<ret;
        return -1;
    }

    // 添加回调（完善 FdContext 类）
    
}

bool IOManager::delEvent(int fd,Event event){

}

bool IOManager::cancelEvent(int fd,Event event){

}

bool IOManager::cancelAllEvent(int fd){

}

void IOManager::tickle(){

}

bool IOManager::stopping(){

}

void IOManager::idle(){

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


}
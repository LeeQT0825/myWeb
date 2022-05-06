#include "fdmanager.h"
#include "hook.h"
#include <sys/stat.h>

namespace myWeb{

FDctx::FDctx(int fd)
        :m_isInit(false)
        ,m_isSocket(false)
        ,m_sysNonblock(false)
        ,m_usrNonblock(false)
        ,m_isClosed(false)
        ,m_fd(fd)
        ,m_recvTimeout(-1)
        ,m_sendTimeout(-1){
    init();
}

bool FDctx::init(){
    if(m_isInit)    return;
    m_recvTimeout=-1;
    m_sendTimeout=-1;

    // 获取句柄状态
    struct stat fd_stat;
    if(fstat(m_fd,&fd_stat)==-1){
        m_isInit=false;
        m_isSocket=false;
    }else{
        m_isInit=true;
        m_isSocket=S_ISSOCK(fd_stat.st_mode);       // 判断是否为 socketfd
    }

    if(m_isSocket){
        int flags=fcntl_f(m_fd,F_GETFL,0);
        if(!(flags & O_NONBLOCK)){
            fcntl_f(m_fd,F_SETFL,flags | O_NONBLOCK);
            m_sysNonblock=true;
        }
    }else{
        m_sysNonblock=false;
    }
    
    m_usrNonblock=false;
    m_isClosed=false;
    return m_isInit;
}

void FDctx::setTimeout(int type,uint64_t timeout){
    if(type==SO_RCVTIMEO){
        m_recvTimeout=timeout;
    }else{
        m_sendTimeout=timeout;
    }
}

uint64_t FDctx::getTimeout(int type){
    if(type==SO_RCVTIMEO){
        return m_recvTimeout;
    }else{
        return m_sendTimeout;
    }
}


FDctx::ptr FDManager::getFD(int fd,bool auto_create){
    if(fd==-1)  return nullptr;
    lock_type::read_lock rd_lck(m_lock);
    if(fd>=(int)m_fds.size()){
        if(!auto_create){
            return nullptr;
        }
    }else{
        if(m_fds[fd] || !auto_create){
            return m_fds[fd];
        }    
    }
    rd_lck.unLock();

    // 创建fd
    lock_type::write_lock wr_lck(m_lock);
    if(fd>=(int)m_fds.size()){
        m_fds.resize(fd*1.5);
    }
    m_fds[fd].reset(new FDctx(fd));

    return m_fds[fd];
}

void FDManager::delFD(int fd){
    lock_type::write_lock wr_lck(m_lock);
    if(fd>=(int)m_fds.size()){
        return;
    }
    m_fds[fd].reset();
}

}
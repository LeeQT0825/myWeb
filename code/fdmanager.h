#ifndef __MYWEB_FDMANAGER_H__
#define __MYWEB_FDMANAGER_H__

#include "mmutex.h"
#include "mthread.h"
#include "iomanager.h"
#include "singleton.h"
#include <vector>

namespace myWeb{

// 文件句柄信息
class FDctx:public std::enable_shared_from_this<FDctx>{
public:
    typedef std::shared_ptr<FDctx> ptr;
    FDctx(int fd);
    ~FDctx();

    // 是否初始化完毕
    bool isInit(){
        return m_isInit;
    }
    // 是否是SocketFD
    bool isSocket(){
        return m_isSocket;
    }
    // 是否已经关闭
    bool isClosed(){
        return m_isClosed;
    }
    // 用户设置（非）阻塞
    void setUsrNonblock(bool flag){
        m_usrNonblock=flag;
    }
    // 获取是否为用户设置非阻塞
    bool getUsrNonblock(){
        return m_usrNonblock;
    }
    // 系统设置（非）阻塞
    void setSysNonblock(bool flag){
        m_sysNonblock=flag;
    }
    // 获取是否为系统设置非阻塞
    bool getSysNonblock(){
        return m_sysNonblock;
    }
    // 设置超时时间 type: SO_RCVTIMEO(接收超时时间)、SO_SNDTIMEO(发送超时时间)
    void setTimeout(int type,uint64_t timeout);
    // 获取超时时间
    uint64_t getTimeout(int type);

private:
    // 初始化
    bool init();

private:
    bool m_isInit;
    bool m_isSocket;
    bool m_sysNonblock;
    bool m_usrNonblock;
    bool m_isClosed;
    int m_fd;
    uint64_t m_recvTimeout;
    uint64_t m_sendTimeout;
    IOManager* m_iomanager;

};

// 文件句柄管理器
class FDManager{
public:
    typedef std::shared_ptr<FDManager> ptr;
    typedef RWmutex lock_type;

    FDManager(){
        m_fds.resize(64);
    }

    // 从管理器中获取句柄，auto_create为true时，如果没有该句柄，则创建一个
    FDctx::ptr getFD(int fd,bool auto_create=false);
    // 删除句柄
    void delFD(int fd);

private:
    lock_type m_lock;
    std::vector<FDctx::ptr> m_fds;
};

}
#endif
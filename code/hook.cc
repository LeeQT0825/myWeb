#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"
#include "macro.h"
#include "config.h"
#include "fdmanager.h"
#include <dlfcn.h>
#include <functional>
#include <stdarg.h>

namespace myWeb{

// 配置connect timeout
static ConfigVar<uint64_t>::ptr config_tcp_connect_timeout=
                Config::Lookup("tcp_connect_timeout",(uint64_t)5000,"tcp connect timeout");

static thread_local bool t_hook_enable=false;

// 宏定义：所有要HOOK的系统调用
#define HOOK_FUNC(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(write) \
    XX(readv) \
    XX(writev) \
    XX(recv) \
    XX(send) \
    XX(recvfrom) \
    XX(sendto) \
    XX(recvmsg) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

// 初始化HOOK
void hook_init(){
    static bool is_inited=false;
    if(is_inited)   return;

    // 获取hook前的系统调用函数，并赋值给函数指针对象
    // 拓展例为： sleep_f=(sleep_func)dlsym(RTLD_NEXT,"sleep");
    #define XX(name) name##_f=(name##_func)dlsym(RTLD_NEXT,#name);
        HOOK_FUNC(XX);
    #undef XX

}


bool is_hook_enable(){
    return t_hook_enable;
}

void set_hook_enable(bool flag){
    t_hook_enable=flag;
}


// 连接超时时间
static uint64_t g_connect_timeout_MS=-1;
// 在main之前就完成hook的实例化
struct _HookIniter{
    _HookIniter(){
        myWeb::hook_init();
        g_connect_timeout_MS=config_tcp_connect_timeout->getVal();
        config_tcp_connect_timeout->addListener([](const uint64_t& oldval,const uint64_t& newval){
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"tcp_connect_timeout changed from "<<oldval<<" to "<<newval;
            g_connect_timeout_MS=newval;
        });
    }
};
static _HookIniter HookIniter; 


// 存放条件定时器的条件：成员 cancelled是errno错误值，如果有错误则停止触发
struct Timer_Cond
{
    int cancelled=0;
};


/* IO 类型函数的hook，失败返回-1
func: 原函数，hook_func_name: hook后的函数名称（写日志用），event: 注册事件，
timeout_so: socket超时时间类型（可选SO_RCVTIMEO(接收超时时间)、SO_SNDTIMEO(发送超时时间)），
args: 原函数参数列表 */
template<typename OriginFunc,typename ... Args>
static ssize_t do_io(int fd,OriginFunc origin_func,const char* hook_func_name,
                uint32_t event,int timeout_so,Args&& ... args){
    if(!myWeb::t_hook_enable){
        return origin_func(fd,std::forward<Args>(args)...);
    }
    // 保证 fd 是非阻塞的，不能auto_create，防止影响原函数对fd的存在性的判断
    myWeb::FDctx::ptr fd_ctx=myWeb::Singleton<myWeb::FDManager>::getInstance()->getFD(fd);
    if(!fd_ctx){
        return origin_func(fd,std::forward<Args>(args)...);
    }
    if(fd_ctx->isClosed()){
        errno=EBADF;
        return -1;
    }
    if(!fd_ctx->isSocket() || fd_ctx->getUsrNonblock()){
        return origin_func(fd,std::forward<Args>(args)...);
    }

    uint64_t time_out=fd_ctx->getTimeout(timeout_so);
    std::shared_ptr<Timer_Cond> timer_cond(new Timer_Cond());

    retry:
    ssize_t n=origin_func(fd,std::forward<Args>(args)...);     //保存返回值，最后再返回

    while(n==-1 && errno==EINTR){

        // 中断错误状态————重试
        n=origin_func(fd,std::forward<Args>(args)...);
    }
    if(n==-1 && errno==EAGAIN){

        // 阻塞错误状态————增加定时器，从 FDctx 中获得 timeout
        myWeb::IOManager* iomanager=myWeb::IOManager::getThis();
        myWeb::Timer::ptr timer;
        std::weak_ptr<Timer_Cond> weak_timercond(timer_cond);

        if(time_out!=(uint64_t)-1){
            // 增加定时器————当到达超时时间时就意味着无论就绪还是阻塞，都要取消事件
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<hook_func_name<<" addTimer";
            timer=iomanager->addCondTimer(time_out,[fd,weak_timercond,iomanager,event](){
                auto cond=weak_timercond.lock();
                if(!cond || cond->cancelled)  return;
                cond->cancelled=ETIMEDOUT;
                iomanager->cancelEvent(fd,(myWeb::IOManager::Event)event);      // 强制触发事件，并返回
            },weak_timercond);
        }
     
        // 注册事件
        int ret=iomanager->addEvent(fd,(myWeb::IOManager::Event)event);
        if(ret){
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<hook_func_name<<" addEvent error";
            if(timer){
                timer->cancelTimer();
            }
            return -1;
        }else{
            myWeb::Fiber::yieldToHold();        // 等定时器被唤醒才会resume
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<hook_func_name<<" resume";
            if(timer){
                timer->cancelTimer();
            }
            if(timer_cond->cancelled){
                errno=timer_cond->cancelled;
                return -1;
            } 
            goto retry;
        }
    }


    return n;
}


// HOOK 函数定义

extern "C"{

// 声明函数指针对象
// 拓展例为： sleep_func sleep_f=nullptr;
#define XX(name) name##_func name##_f=nullptr;
    HOOK_FUNC(XX);
#undef XX

unsigned int sleep(unsigned int seconds){
    if(!myWeb::t_hook_enable)   return sleep_f(seconds);

    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"sleep yield";
    myWeb::Fiber::ptr fiber=myWeb::Fiber::getThis();
    myWeb::IOManager* iomanager=myWeb::IOManager::getThis();
    iomanager->addTimer((uint64_t)seconds*1000,std::bind(
                (void(myWeb::Scheduler::*)(myWeb::Fiber::ptr,pid_t))&myWeb::IOManager::schedule,
                iomanager,fiber,-1));       // TODO

    myWeb::Fiber::yieldToHold();
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"sleep resume";

    return 0;
}

int usleep(unsigned int usec){
    if(!myWeb::t_hook_enable)   return usleep_f(usec);

    myWeb::Fiber::ptr fiber=myWeb::Fiber::getThis();
    myWeb::IOManager* iomanager=myWeb::IOManager::getThis();
    iomanager->addTimer((uint64_t)usec/1000,std::bind(
                (void(myWeb::Scheduler::*)(myWeb::Fiber::ptr,pid_t))&myWeb::IOManager::schedule,
                iomanager,fiber,-1));
                
    myWeb::Fiber::yieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem){
    if(!myWeb::t_hook_enable)   return nanosleep_f(req,rem);

    int timeout_ms=req->tv_sec*1000+rem->tv_nsec/1000/1000;
    myWeb::Fiber::ptr fiber=myWeb::Fiber::getThis();
    myWeb::IOManager* iomanager=myWeb::IOManager::getThis();
    iomanager->addTimer(timeout_ms,std::bind(
                (void(myWeb::Scheduler::*)(myWeb::Fiber::ptr,pid_t))&myWeb::IOManager::schedule,
                iomanager,fiber,-1));
                
    myWeb::Fiber::yieldToHold();
    return 0;    
}

int socket(int domain, int type, int protocol){
    if(!myWeb::t_hook_enable){
        return socket_f(domain,type,protocol);
    }
    int fd=socket_f(domain,type,protocol);
    if(fd>=0){
        myWeb::Singleton<myWeb::FDManager>::getInstance()->getFD(fd,true);
    }
    return fd;
}

// 超时连接
int connect_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen,uint64_t timeout_ms){
    if(!myWeb::t_hook_enable){
        return connect_f(sockfd,addr,addrlen);
    }
    myWeb::FDctx::ptr fd_ctx=myWeb::Singleton<myWeb::FDManager>::getInstance()->getFD(sockfd);
    if(!fd_ctx || fd_ctx->isClosed()){
        errno=EBADF;
        return -1;
    }
    if(!fd_ctx->isSocket() || fd_ctx->getUsrNonblock()){
        return connect_f(sockfd,addr,addrlen);
    }

    int ret=connect_f(sockfd,addr,addrlen);
    if(ret==0){
        return 0;
    }else if(errno!=EINPROGRESS){       // 套接字本身为非阻塞，返回-1，但连接还在继续
        return -1;
    }

    myWeb::IOManager* iomanager=myWeb::IOManager::getThis();
    myWeb::Timer::ptr timer;
    std::shared_ptr<Timer_Cond> timer_cond(new Timer_Cond());
    std::weak_ptr<Timer_Cond> weak_timerCond(timer_cond);

    // 添加定时器
    if(timeout_ms!=(uint64_t)-1){
        timer=iomanager->addCondTimer(timeout_ms,[weak_timerCond,iomanager,sockfd](){
            auto tcond=weak_timerCond.lock();
            if(!tcond || tcond->cancelled){
                return;
            }
            tcond->cancelled=ETIMEDOUT;
            iomanager->cancelEvent(sockfd,myWeb::IOManager::Event::WRITE);
        },weak_timerCond);
    }

    // 注册事件
    ret=iomanager->addEvent(sockfd,myWeb::IOManager::Event::WRITE);
    if(ret==0){
        myWeb::Fiber::yieldToHold();
        if(timer){
            timer->cancelTimer();
        }
        if(timer_cond->cancelled){
            errno=timer_cond->cancelled;
            return -1;
        }
    }else{
        if(timer){
            timer->cancelTimer();
        }
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"connect_timeout addEvent error";
    }

    int error=0;
    socklen_t len=sizeof(int);
    if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len)!=-1){
        if(!error)  return 0;
        errno=error;
    }
    return -1;

}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    return connect_timeout(sockfd,addr,addrlen,g_connect_timeout_MS);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    // 返回新的 fd 
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"accept";
    int fd=do_io(sockfd,accept_f,"accept",myWeb::IOManager::Event::READ,SO_RCVTIMEO,addr,addrlen);
    if(fd>=0){
        myWeb::Singleton<myWeb::FDManager>::getInstance()->getFD(fd,true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count){
    return fd=do_io(fd,read_f,"read",myWeb::IOManager::Event::READ,SO_RCVTIMEO,buf,count);
}

ssize_t write(int fd, const void *buf, size_t count){
    return do_io(fd,write_f,"write",myWeb::IOManager::Event::WRITE,SO_SNDTIMEO,buf,count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt){
    return do_io(fd,readv_f,"readv",myWeb::IOManager::Event::READ,SO_RCVTIMEO,iov,iovcnt);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt){
    return do_io(fd,writev_f,"writev",myWeb::IOManager::Event::WRITE,SO_SNDTIMEO,iov,iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags){
    return do_io(sockfd,recv_f,"recv",myWeb::IOManager::Event::READ,SO_RCVTIMEO,buf,len,flags);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags){
    return do_io(sockfd,send_f,"send",myWeb::IOManager::Event::WRITE,SO_SNDTIMEO,buf,len,flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
    return do_io(sockfd,recvfrom_f,"recvfrom",myWeb::IOManager::Event::READ,SO_RCVTIMEO,buf,len,flags,src_addr,addrlen);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen){
    return do_io(sockfd,sendto_f,"sendto",myWeb::IOManager::Event::WRITE,SO_SNDTIMEO,buf,len,flags,dest_addr,addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags){
    return do_io(sockfd,recvmsg_f,"recvmsg",myWeb::IOManager::Event::READ,SO_RCVTIMEO,msg,flags);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags){
    return do_io(sockfd,sendmsg_f,"sendmsg",myWeb::IOManager::Event::WRITE,SO_SNDTIMEO,msg,flags);
}

// TODO: 这里存疑，close不应该是shutdown，应该是将结点引用-1
int close(int fd){
    if(!myWeb::t_hook_enable){
        return close_f(fd);
    }
    myWeb::FDctx::ptr fd_ctx=myWeb::Singleton<myWeb::FDManager>::getInstance()->getFD(fd);
    if(!fd_ctx){
        myWeb::IOManager* iomanager=myWeb::IOManager::getThis();
        if(iomanager){
            iomanager->cancelAllEvent(fd);
        }
        myWeb::Singleton<myWeb::FDManager>::getInstance()->delFD(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... ){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"fcntl";
    va_list va;
    va_start(va,cmd);
    switch(cmd){
        case F_SETFL:
            {
                int arg=va_arg(va,int);
                va_end(va);
                myWeb::FDctx::ptr fd_ctx=myWeb::Singleton<myWeb::FDManager>::getInstance()->getFD(fd);
                if(!fd_ctx || fd_ctx->isClosed() || !fd_ctx->isSocket()){
                    return fcntl_f(fd,cmd,arg);
                }
                fd_ctx->setUsrNonblock(arg & O_NONBLOCK);
                return fcntl_f(fd,cmd,arg);
            }
            break;
        
        // int 类型
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
        case F_SETPIPE_SZ:
            {
                int arg=va_arg(va,int);
                va_end(va);
                return fcntl_f(fd,cmd,arg);
            }

        // void 类型
        case F_GETFL:
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
        case F_GETPIPE_SZ:
            {
                va_end(va);
                return fcntl_f(fd,cmd);
            }
            break;

        // flock* 类型
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                flock* arg=va_arg(va,flock*);
                va_end(va);
                return fcntl(fd,cmd,arg);
            }
            break;

        // f_owner_ex* 类型
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                f_owner_ex* arg=va_arg(va,f_owner_ex*);
                va_end(va);
                return fcntl(fd,cmd,arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd,cmd);
    }
}

// 管理IO通道
int ioctl(int fd, unsigned long request, ... ){
    va_list va;
    va_start(va,request);
    void* arg=va_arg(va,void*);
    va_end(va);

    // 设置/ 清除非阻塞I/O 标志
    if(request==FIONBIO){
        bool user_nonblock=!!*(int*)arg;
        myWeb::FDctx::ptr fd_ctx=myWeb::Singleton<myWeb::FDManager>::getInstance()->getFD(fd);
        if(!fd_ctx || fd_ctx->isClosed() || !fd_ctx->isSocket()){
            return ioctl_f(fd,request,arg);
        }
        fd_ctx->setUsrNonblock(user_nonblock);
    }
    return ioctl_f(fd,request,arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen){
    return getsockopt_f(sockfd,level,optname,optval,optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen){
    if(!myWeb::t_hook_enable){
        return setsockopt_f(sockfd,level,optname,optval,optlen);
    }
    if(level==SOL_SOCKET){
        if(optname==SO_RCVTIMEO || optname==SO_SNDTIMEO){
            myWeb::FDctx::ptr fd_ctx=myWeb::Singleton<myWeb::FDManager>::getInstance()->getFD(sockfd);
            if(fd_ctx){
                const timeval* timeout=(const timeval*)optval;
                fd_ctx->setTimeout(optname,timeout->tv_sec*1000+timeout->tv_usec/1000);
            }
        }
    }
    return setsockopt_f(sockfd,level,optname,optval,optlen);
}

}
}
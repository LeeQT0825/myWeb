#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"
#include "macro.h"
#include "config.h"
#include <dlfcn.h>
#include <functional>

namespace myWeb{

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

// 在main之前就完成hook的实例化
struct _HookIniter{
    _HookIniter(){
        hook_init();
    }
};
static _HookIniter HookIniter; 

bool is_hook_enable(){
    return t_hook_enable;
}

void set_hook_enable(bool flag){
    t_hook_enable=flag;
}

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

}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){

}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){

}

ssize_t read(int fd, void *buf, size_t count){

}

ssize_t write(int fd, const void *buf, size_t count){

}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt){

}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt){

}

ssize_t recv(int sockfd, void *buf, size_t len, int flags){

}

ssize_t send(int sockfd, const void *buf, size_t len, int flags){

}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){

}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen){

}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags){

}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags){

}

int close(int fd){

}

int fcntl(int fd, int cmd, ...){

}

int ioctl(int fd, unsigned long request, ...){

}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen){

}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen){
    
}

}
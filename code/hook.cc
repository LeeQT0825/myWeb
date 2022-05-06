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
    XX(usleep)

    // XX(nanosleep)
    // XX(socket)
    // XX(connect)
    // XX(accept)
    // XX(read)
    // XX(write)
    // XX(readv)
    // XX(writev)
    // XX(recv)
    // XX(send)
    // XX(recvfrom)
    // XX(sendto)
    // XX(recvmsg)
    // XX(sendmsg)
    // XX(close)
    // XX(fcntl)
    // XX(ioctl)
    // XX(getsockopt)
    // XX(setsockopt) 

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
    if(!myWeb::t_hook_enable)   return sleep_f(usec);

    myWeb::Fiber::ptr fiber=myWeb::Fiber::getThis();
    myWeb::IOManager* iomanager=myWeb::IOManager::getThis();
    iomanager->addTimer((uint64_t)usec/1000,std::bind(
                (void(myWeb::Scheduler::*)(myWeb::Fiber::ptr,pid_t))&myWeb::IOManager::schedule,
                iomanager,fiber,-1));
                
    myWeb::Fiber::yieldToHold();
    return 0;
}


}
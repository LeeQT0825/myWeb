#include "mthread.h"
#include "log.h"
#include "util.h"

namespace myWeb{

static thread_local Thread* t_thread=nullptr;
static thread_local std::string t_thread_name="UNKNOW";

Thread::Thread(std::function<void()> cb,const std::string& name)
    :m_cb(cb){
    if(name.empty()){
        m_name="UNKNOW";
    }else{
        m_name=name.substr(0,15);
    }
    int ret=pthread_create(&m_thread,nullptr,&Th_Create,this);    // 成功返回0，失败返回其他
    if(ret){
        INLOG_ERROR(MYWEB_ROOT_LOG)<<"pthread_create failed,name: "<<name;
        throw std::logic_error("pthread_create error");
    }
}

Thread::~Thread(){
    if(m_thread){
        pthread_detach(m_thread);
    }
}

void Thread::join(){
    // 如果 m_thread 有值，则说明线程未结束
    if(m_thread){
        int ret=pthread_join(m_thread,nullptr);
        if(ret){
            INLOG_ERROR(MYWEB_ROOT_LOG)<<"pthread_join failed,tid: "<<m_thread;
            throw std::logic_error("pthread_join error");
        }
        m_thread=0;
    }
}

void* Thread::Th_Create(void* args){
    // 静态函数作为新线程的执行入口，所以类对象仍是父线程作用域中的。
    // pthread_create()第三个参数类型是void* (*start_routine)(void*)，所以这里参数类型也是void*
    Thread* th=(Thread*)args;
    t_thread=th;
    t_thread_name=th->m_name;
    th->m_id=myWeb::GetThreadID();      // 获取内核线程
    // 可以给别的线程设置名称，字段长超过16字符（包含'\0'）就会报错
    int ret=pthread_setname_np(pthread_self(),th->m_name.substr(0,15).c_str());      
    if(ret){
        INLOG_DEBUG(MYWEB_ROOT_LOG)<<"pthread name invalid: "<<th->m_name;
        throw std::logic_error("pthread name invalid"); 
    }

    std::function<void()> func=th->m_cb;
    func();     // 执行函数入口
    return 0;
}

Thread* Thread::getThisThread(){
    return t_thread;
}

const std::string& Thread::getThisThreadName(){
    return t_thread_name;
}

void Thread::setThisThreadName(const std::string& name){
    if(name.empty())    return;
    if(t_thread){
        t_thread->m_name=name;
    }
    t_thread_name=name;
}



}
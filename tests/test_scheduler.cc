#include "../code/myweb.h"
#include <iostream>
#include <vector>



void func1();
void func2();
void func3();

void func(){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"func";
}

void func1(){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"func_1";
    // sleep(1);
    // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"func1 awake";
}

void func2(){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"func_2";
    // myWeb::Fiber::yieldToHold();
    // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"func_2 back";
}

void func3(){  
    static int count=5;
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"func_3: "<<count;
    if(--count>=0){
        sleep(1);
        myWeb::Scheduler::getScheduler()->schedule(&func3);
    }
}

void func4(){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"func_4";
}
int main(){
    LOADYAML;

    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"Main ThreadID: "<<myWeb::GetThreadID();
    std::cout<<"-------------------------------------------------"<<std::endl;

    // myWeb::Fiber::ptr mainThr_rootFiber=myWeb::Fiber::getThis();
    // 创建调度器 
    myWeb::Scheduler::ptr sche(new myWeb::Scheduler(5,false,"sche"));
    // 创建执行任务
    myWeb::Fiber::ptr work_fiber(new myWeb::Fiber(func));
    std::vector<std::function<void()> > func_vec{func1,func2,func3};

    sche->schedule(func_vec.begin(),func_vec.end());
    // sche->schedule(work_fiber,myWeb::GetThreadID());    // 指定调度线程执行

    sche->start();
    sche->schedule(&func4);

    sche->stop();

    return 0;
}
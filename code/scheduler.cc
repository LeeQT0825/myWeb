#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace myWeb{

// 调度器
static thread_local Scheduler* t_Master_Scheduler=nullptr;
// 调度协程————执行run（每个执行线程里都有一个）
static thread_local Fiber* t_Master_Fiber=nullptr;

// 静态函数

Scheduler* Scheduler::getScheduler(){
    return t_Master_Scheduler;
}

Fiber* Scheduler::getMasterFiber(){
    return t_Master_Fiber;
}


// 成员函数

Scheduler::Scheduler(size_t num_of_thread,bool use_caller,const std::string& name)
        :m_name(name){
    MYWEB_ASSERT(num_of_thread>0);
    MYWEB_ASSERT_2(getScheduler()==nullptr,"Scheduler already exist");
    if(use_caller){
        --num_of_thread;
        Fiber::getThis();
        t_Master_Scheduler=this;     // 当前线程的调度器
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,false));       
        t_Master_Fiber=m_rootFiber.get();       // 当前线程的调度协程
        m_rootThreadID=GetThreadID();
        m_ThreadIDs.push_back(m_rootThreadID);
    }else{
        m_rootThreadID=-1;
    }
    m_ThrPoolCount=num_of_thread;
}
Scheduler::~Scheduler(){
    MYWEB_ASSERT(!m_running); 
    if(getScheduler()==this){
        t_Master_Scheduler=nullptr;
    }
}

void Scheduler::setThis(){
    t_Master_Scheduler=this;
}

void Scheduler::tickle(){
    // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"tickle";
}

void Scheduler::run(){
    setThis();
    set_hook_enable(true);
    if(GetThreadID()!=m_rootThreadID){
        t_Master_Fiber=Fiber::getThis().get();
    }

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));      // 如果调度任务执行完，要执行的东西
    Fiber::ptr cb_fiber;        // 承接 FiberAndThread::func

    FiberAndThread ft;
    while (1)
    {
        bool tickle_me=false;
        bool is_active=false;
        ft.reset();

        // 从执行队列中取出
        {   
            lock_type::scoped_lock lck(m_lock);
            auto iter=m_executions.begin();
            while(iter!=m_executions.end()){
                // 处理指定当前线程的执行任务
                if(iter->thread!=-1 && iter->thread!=GetThreadID()){
                    ++iter;
                    tickle_me=true;
                    continue;
                }
                MYWEB_ASSERT_2(iter->fiber || iter->func,"executions error");
                if(iter->fiber && iter->fiber->getState()==Fiber::State::EXEC){
                    // 已经在执行   TODO 为什么不从队列中删除？
                    ++iter;
                    continue;
                }
                ft=*iter;
                m_executions.erase(iter++);
                ++m_activeThreadCount;
                is_active=true;
                break;
            }
            tickle_me |= iter!=m_executions.end();
        }
        
        if(tickle_me){
            // 通知其他线程
            tickle();
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"tickle in run";
        }

        // 待执行的是协程
        if(ft.fiber && (ft.fiber->getState()!=Fiber::State::TERM 
                    && ft.fiber->getState()!=Fiber::State::EXCEPT)){
            ft.fiber->swapIn();
            --m_activeThreadCount;

            if(ft.fiber->getState()==Fiber::State::READY){
                schedule<Fiber::ptr>(ft.fiber);     // 任意线程都可以
            }else if(ft.fiber->getState()!=Fiber::State::TERM 
                        && ft.fiber->getState()!=Fiber::State::EXCEPT){
                ft.fiber->m_state=Fiber::State::HOLD;
            }
        }else if(ft.func){

            // 待执行的是函数
            if(cb_fiber){
                cb_fiber->reset(ft.func);
            }else{
                cb_fiber.reset(new Fiber(ft.func));
            }
            cb_fiber->swapIn();
            --m_activeThreadCount;

            if(cb_fiber->getState()==Fiber::State::READY){
                schedule<Fiber::ptr>(cb_fiber);     // 任意线程都可以
            }else if(cb_fiber->getState()==Fiber::State::TERM 
                        || cb_fiber->getState()==Fiber::State::EXCEPT){
                cb_fiber->reset(nullptr);           // 防止该协程析构
            }else{
                cb_fiber->m_state=Fiber::State::HOLD;
                cb_fiber.reset();                   // 不丢掉原来指向的fiber，重新添加一个fiber，这里很细节~
            }
        }else{
            if(is_active){  
            // 执行队列未空，但未匹配到任务
                --m_activeThreadCount;
                continue;
            }

            // 执行队列为空，检查是否是处于关闭状态（当 ft.fiber 的状态是 TERM 或者 EXCEPT 的时候）
            if(idle_fiber->getState()==Fiber::State::TERM){
                INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"idle fiber terminate";
                break;
            }
            ++m_idleThreadCount;
            idle_fiber->swapIn();
            // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"idle out";
            --m_idleThreadCount;
            if(idle_fiber->getState()!=Fiber::State::TERM 
                    && idle_fiber->getState()!=Fiber::State::EXCEPT){
                idle_fiber->m_state=Fiber::State::HOLD;
            }
        }
    }
}

void Scheduler::start(){
    lock_type::scoped_lock lck(m_lock);
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"Scheduler start";

    if(m_running){
        return;
    }
    m_running=true;
    MYWEB_ASSERT(m_Threadpool.empty());

    // 创建线程池，固定大小
    m_Threadpool.resize(m_ThrPoolCount);
    for(size_t i=0;i<m_ThrPoolCount;++i){
        m_Threadpool[i].reset(new Thread(std::bind(&Scheduler::run,this),m_name+"_"+std::to_string(i)));
        m_ThreadIDs.push_back(m_Threadpool[i]->getID());
    }
    lck.unLock();
}

void Scheduler::stop(){
    // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"stop()";
    m_self_stopping=true;

    // 直接停止的条件
    if(m_rootFiber && m_ThrPoolCount==0
                    && (m_rootFiber->getState()==Fiber::State::TERM
                    || m_rootFiber->getState()==Fiber::State::INIT)){
        m_running=false;
        if(isStopping()){
            return;
        }
    }

    if(m_rootThreadID==-1){
        // 非 use_caller 模式
        MYWEB_ASSERT(getScheduler()!=this);
    }else{
        // use_caller 模式————必须在调用 Schedule 类的线程中停止
        MYWEB_ASSERT(getScheduler()==this);
    }
    m_running=false;

    // 处理主调度协程
    if(m_rootFiber){
       if(!isStopping()){
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"m_rootFiber come";
            m_rootFiber->call();
        }    
    }
    while(m_rootFiber && m_rootFiber->getState()!=Fiber::State::TERM 
                && m_rootFiber->getState()!=Fiber::State::INIT 
                && m_rootFiber->getState()!=Fiber::State::EXCEPT){
        if(!isStopping()){
            m_rootFiber->call();
        }
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"tickle in stop to rootFiber";
    }

    // 唤醒线程，跑完剩下的任务
    if(m_activeThreadCount){
        tickle();
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"tickle in stop to threads";
    }

    // 释放线程池
    std::vector<Thread::ptr> thrs;
    {
        lock_type::scoped_lock lck(m_lock);
        thrs.swap(m_Threadpool);
        m_ThrPoolCount=0;
    }
    for(auto i:thrs){
        i->join();
    }
    
    // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"m_rootFiber state: "<<m_rootFiber->getState();
}

bool Scheduler::isStopping(){
    lock_type::scoped_lock lck(m_lock);
    // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"isStopping: "<<!m_running<<m_self_stopping<<m_executions.empty()<<m_activeThreadCount;
    return (!m_running) && m_self_stopping && m_executions.empty() && m_activeThreadCount==0;
}

void Scheduler::idle(){
    while(!isStopping()){
        Fiber::yieldToHold();
    }
}


}
#ifndef __MYWEB_SCHEDULER_H__
#define __MYWEB_SCHEDULER_H__

#include <memory>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include "fiber.h"
#include "mmutex.h"
#include "mthread.h"


namespace myWeb{

// 协程调度器———可调度 Fiber 或 function 到指定线程中执行
class Scheduler{
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef MutexLock lock_type;
    
    // use_caller ：是否将调用 Schedule 对象的当前线程加入到线程池
    Scheduler(size_t num_of_thread=1,bool use_caller=true,const std::string& name=" ");
    virtual ~Scheduler();

    // 获取调度器名称
    const std::string& getName() const{
        return m_name;
    }
    // 启动协程调度器
    void start();
    // 主动停止协程调度器（如果有未执行完的任务，要等到它们都完成后才退出）
    void stop();


    // 添加到执行队列  (thread参数表示目标线程，-1表示任意线程)
    template<typename EXECS>
    void schedule(EXECS fc,pid_t thread=-1){
        bool need_tickle=false;      // 当添加前队列为空，则tickle提醒来新的执行操作了
        // 上锁
        {
            lock_type::scoped_lock lck(m_lock);
            need_tickle=schedule_nolock(fc,thread);
        }
        if(need_tickle){
            tickle();
        }
    }
    // 批量添加到执行队列，目标线程默认-1
    template<typename INPUT_ITER>
    void schedule(INPUT_ITER begin_iter,INPUT_ITER end_iter){
        bool need_tickle=false;
        {
            lock_type::scoped_lock lck(m_lock);
            for(;begin_iter!=end_iter;++begin_iter){
                if(schedule_nolock(&*begin_iter,-1)){       // 防止迭代器失效，所以用指针，把所指对象swap掉
                    need_tickle=true;
                }
            }
        }
        if(need_tickle){
            tickle();
        }
    }

    // 获取当前调度器
    static Scheduler* getScheduler();
    // 返回当前协程调度器的调度协程
    static Fiber* getMasterFiber();

protected:
    // 设置当前调度器
    void setThis();
    // 执行队列中新加入执行任务时，提醒线程池中所有线程以及 m_rootFiber 执行run
    virtual void tickle();
    // 调度器执行入口
    void run();
    // 子类实现具体清理操作
    virtual bool stopping();
    // 空闲任务 (调度器又没任务可调度，又不能使线程终止时的操作————忙等待或者sleep)
    virtual void idle();

private:
    // 添加到执行队列，如果添加前队列为空，则返回提醒
    template<typename EXECS>
    bool schedule_nolock(EXECS fc,pid_t thread=-1){
        bool need_tickle=m_executions.empty();
        FiberAndThread toExc(fc,thread);
        if(toExc.fiber || toExc.func){
            m_executions.push_back(toExc);
        }
        return need_tickle;
    }
  
private:
    // 可被调度的结构体
    typedef struct FiberAndThread{
        FiberAndThread(){
            thread=-1;
        }

        FiberAndThread(Fiber::ptr fiberptr,pid_t threadID)
            :fiber(fiberptr)
            ,thread(threadID){}

        FiberAndThread(Fiber::ptr* fiberptr,pid_t threadID)
            :thread(threadID){
                fiber.swap(*fiberptr);
        }

        FiberAndThread(std::function<void()> cb,pid_t threadID)
            :func(cb)
            ,thread(threadID){}

        FiberAndThread(std::function<void()>* cb,pid_t threadID)
            :thread(threadID){
                func.swap(*cb);
        }

        void reset(){
            fiber=nullptr;
            func=nullptr;
            thread=-1;
        }

        Fiber::ptr fiber;
        std::function<void()> func;
        pid_t thread;     // 内核线程ID
    }FiberAndThread;
    
private:
    lock_type m_lock;
    // 线程池
    std::vector<Thread::ptr> m_Threadpool;
    // 待执行队列（可以是 function 或 fiber）
    std::list<FiberAndThread> m_executions;
    // 调度器名称
    std::string m_name;
    // 调度协程
    Fiber::ptr m_rootFiber=nullptr;
    // 调度线程ID(use_caller) 
    pid_t m_rootThreadID=-1;
    
protected:
    // 线程池中所有的线程ID
    std::vector<pid_t> m_ThreadIDs;
    // 线程池大小
    size_t  m_ThrPoolCount=0;
    // 活跃线程数量
    std::atomic<size_t> m_activeThreadCount{0};
    // 空闲线程数量
    std::atomic<size_t> m_idleThreadCount{0};
    // 是否正在运行
    bool m_running=false;
    // 是否主动停止
    bool m_self_stopping=false;
    
};


}

#endif
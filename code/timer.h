#ifndef __MYWEB_TIMER__
#define __MYWEB_TIMER__

#include <memory>
#include <functional>
#include <set>
#include <vector>
#include "mmutex.h"

namespace myWeb{

class TimerManager;

// 定时器（唯一的构造入口在TimeManager中）
class Timer:public std::enable_shared_from_this<Timer>{
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;

    // 取消定时器 false: 早已被取消掉了
    bool cancelTimer();
    // 刷新定时器
    bool refreshTimer();
    // 重置定时器
    bool resetTimer(uint64_t ms,bool from_now);

private:
    // 只能通过TimeManager内部构造（ms:定时间隔时间，iscycle：是否循环，manager：定时器管理器）
    Timer(uint64_t ms,std::function<void()> cb,bool iscycle,TimerManager* manager);
    // 仅用于获取当前时间，以在 set 中比较
    Timer(uint64_t next);
    struct Comparator{
        // 传入参数是智能指针
        bool operator()(const Timer::ptr& A,const Timer::ptr& B) const ;
    };

private:
    // 定时间隔时间
    uint64_t m_ms=0;
    // 执行的绝对时间
    uint64_t m_nextTime=0;
    // 回调
    std::function<void()> m_cb;
    // 是否循环
    bool m_isCycle;
    // 管理器指针
    TimerManager* m_manager=nullptr;

};

// 定时器管理器
class TimerManager{
friend class Timer;
public:
    typedef RWmutex lock_type;

    TimerManager();
    virtual ~TimerManager(){}

    // 添加定时器（ms:定时间隔时间，iscycle：是否循环）
    Timer::ptr addTimer(uint64_t ms,std::function<void()> cb,bool iscycle=false);
    // 添加定时器（传入指针）
    void addTimer(Timer::ptr timer,lock_type::write_lock wr_lck);
    // 添加条件定时器——当超时时，仅当条件存在的时候才会触发（ms:定时间隔时间，weak_cond：触发条件，iscurring：是否循环）
    Timer::ptr addCondTimer(uint64_t ms,std::function<void()> cb,std::weak_ptr<void> weak_cond,bool iscycle=false);
    // 获取下一个定时器的时间间隔
    uint64_t getNextTime();
    // 获取所有到时定时器的执行函数（取出定时器）
    void ExpiredCB(std::vector<std::function<void()> >& cbs);

protected:
    // 当新插入的定时器排在最前面的时候，需要通知 epoll_wait 修改其超时时间
    virtual void frontTimerChanged() = 0;

private:
    // 时间校准（检测是否修改了系统时间）
    // bool detectClkRollOver(uint64_t now_time);

private:
    lock_type m_lock;
    std::set<Timer::ptr,Timer::Comparator> m_timers;
    bool is_tickled=false;      // epoll_wait 修改一次时间期间只允许唤醒一次（唤醒多了也没用，还是要等 epoll_wait）
    // uint64_t m_pretime=0;         // 上次执行时间

};


}

#endif
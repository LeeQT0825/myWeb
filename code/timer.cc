#include "timer.h"
#include "util.h"

namespace myWeb{

Timer::Timer(uint64_t ms,std::function<void()> cb,bool iscycle,TimerManager* manager)
        :m_ms(ms)
        ,m_cb(cb)
        ,m_isCycle(iscycle)
        ,m_manager(manager){
    m_nextTime=myWeb::GetCurrentMS()+m_ms;
}
Timer::Timer(uint64_t next)
        :m_nextTime(next){}

bool Timer::cancelTimer(){
    if(m_cb){
        TimerManager::lock_type::write_lock wr_lck(m_manager->m_lock);
        m_cb=nullptr;
        auto iter=m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(iter);
        return true;
    }
    return false;
}

bool Timer::refreshTimer(){
    if(!m_cb){
        return false;
    }
    TimerManager::lock_type::write_lock wr_lck(m_manager->m_lock);
    // 调整 TimerManager ，删除重新加入(不能直接修改key)
    auto iter=m_manager->m_timers.find(shared_from_this());
    if(iter==m_manager->m_timers.end()){
        return false;
    }
    m_manager->m_timers.erase(iter);
    m_nextTime=GetCurrentMS()+m_ms;
    m_manager->addTimer(shared_from_this(),wr_lck);
    return true;
}

bool Timer::resetTimer(uint64_t ms,bool from_now){
    if(m_ms==ms && !from_now){
        return true;
    }
    if(!m_cb){
        return false;
    }

    TimerManager::lock_type::write_lock wr_lck(m_manager->m_lock);
    auto iter=m_manager->m_timers.find(shared_from_this());
    if(iter==m_manager->m_timers.end()){
        return false;
    }
    m_manager->m_timers.erase(iter);

    if(from_now){
        m_ms=ms;
        m_nextTime=GetCurrentMS()+m_ms;
    }else{
        m_nextTime+=(int64_t)(ms-m_ms);
        m_ms=ms;
    }

    m_manager->addTimer(shared_from_this(),wr_lck);
    return true;
}

bool Timer::Comparator::operator()(const Timer::ptr& A,const Timer::ptr& B) const {
    if(!A && !B)    return false;
    if(!A)          return true;
    if(!B)          return false;
    if(A->m_nextTime < B->m_nextTime)   return true;
    if(A->m_nextTime > B->m_nextTime)   return false;
    return A.get() < B.get();
}


TimerManager::TimerManager(){
    // m_pretime=GetCurrentMS();
}

Timer::ptr TimerManager::addTimer(uint64_t ms,std::function<void()> cb,bool iscycle){
    Timer::ptr timer(new Timer(ms,cb,iscycle,this));
    lock_type::write_lock wr_lck(m_lock);

    addTimer(timer,wr_lck);
    return timer;
}

void TimerManager::addTimer(Timer::ptr timer,lock_type::write_lock& wr_lck){
    auto iter=m_timers.insert(timer).first;
    bool at_front=(iter==m_timers.begin()) && !is_tickled;
    is_tickled=true;
    wr_lck.unLock();

    if(at_front){
        frontTimerChanged(); 
    }
}

// addCondTimer() 辅助函数
static void OnTime(std::weak_ptr<void> weak_cond,std::function<void()> cb){
    std::shared_ptr<void> temp=weak_cond.lock();
    if(temp){
        cb();
    }
}
// 如果 weak_ptr 未释放则执行
Timer::ptr TimerManager::addCondTimer(uint64_t ms,std::function<void()> cb,
                                std::weak_ptr<void> weak_cond,bool iscycle){
    return addTimer(ms,std::bind(&OnTime,weak_cond,cb),iscycle);
}

uint64_t TimerManager::getNextTime(){
    lock_type::read_lock rd_lck(m_lock);
    is_tickled=false;
    if(m_timers.empty()){
        return ~0ull;
    }

    const Timer::ptr& front_timer=*m_timers.begin();
    uint64_t now=GetCurrentMS();
    if(now>=front_timer->m_nextTime){
        return 0;
    }
    return front_timer->m_nextTime-now;
}

void TimerManager::ExpiredCB(std::vector<std::function<void()> >& cbs){
    uint64_t now=GetCurrentMS();
    std::vector<Timer::ptr> expiredtimer;
    {
        lock_type::read_lock rd_lck(m_lock);
        if(m_timers.empty()){
            return;
        }
    }
    // bool rollover=detectClkRollOver(now);
    // if(!rollover && (*m_timers.begin())->m_nextTime>now){
    //     return;
    // }

    lock_type::write_lock wr_lck(m_lock);
    Timer::ptr now_timer(new Timer(now));           // 用于在 set 中比较
    auto iter=m_timers.upper_bound(now_timer);      // 获取大于 now_timer 的位置
    expiredtimer.insert(expiredtimer.begin(),m_timers.begin(),iter);
    m_timers.erase(m_timers.begin(),iter);

    for(auto& timer:expiredtimer){
        cbs.push_back(timer->m_cb);
        if(timer->m_isCycle){
            timer->m_nextTime=now+timer->m_ms;
            addTimer(timer,wr_lck);
        }else{
            timer->m_cb=nullptr;                    // 防止回调函数里有些智能指针
        }
    }
}

// bool TimerManager::detectClkRollOver(uint64_t now_time){
//     bool rollover=false;
//     if(now_time<m_pretime && now_time<(m_pretime-1000*60*60)){
//         rollover=true;
//     }
//     m_pretime=now_time;
//     return rollover;
// }

}
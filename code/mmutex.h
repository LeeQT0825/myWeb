#ifndef __MYWEB_MMUTEX_H__
#define __MYWEB_MMUTEX_H__

#include <semaphore.h>
#include <stdint.h>
#include "log.h"

namespace myWeb{

// 信号量    
class Semaphore{
public:
    Semaphore(uint32_t count=0);
    ~Semaphore();

    // 获取信号量
    void wait();
    // 释放信号量
    void post();

private:
    Semaphore(const Semaphore& )=delete;
    Semaphore(const Semaphore&& )=delete;
    Semaphore& operator=(const Semaphore& )=delete;

private:
    sem_t m_semaphore;
};

// 锁：应满足RAII，防止线程在占用资源后，忘记释放锁造成资源泄露

// 自动加锁，自动解锁（模仿unique_lock<>）。用来防止未及时释放锁造成的资源泄露的发生。T是某种锁的类型
template<typename T>
class ScopedLockImpl{
public:
    ScopedLockImpl(T& mutex)
        :m_mutex(m_mutex){
        m_mutex.lock();
        m_locked=true;
    }
    ~ScopedLockImpl(){
        unLock();
    }

    void Lock(){
        if(!m_locked){
            m_mutex.lock();
            m_locked=true;
        }
    }
    void unLock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked=false;
        }
    }
private:
    T m_mutex;
    bool m_locked=false;
};


}
#endif
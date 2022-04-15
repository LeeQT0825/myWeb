#ifndef __MYWEB_MMUTEX_H__
#define __MYWEB_MMUTEX_H__

#include <semaphore.h>
#include <stdint.h>
#include <mutex>
#include <pthread.h>
#include <atomic>

namespace myWeb{

// 信号量(初始值为0)    
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

// （这段的核心）封装各种锁：应满足RAII，防止线程在占用资源后，忘记释放锁造成资源泄露

// 自动加锁，自动解锁（模仿unique_lock<>）。用来防止未及时释放锁造成的资源泄露的发生。T是某种锁的类型
template<typename T>
class ScopedLockImpl{
public:
    ScopedLockImpl(T& mutex)
        :m_mutex(mutex){
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
    T& m_mutex;
    bool m_locked=false;
};

// 读锁
template<typename T>
class ReadScopedLockImpl{
public:
    ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex){
        m_mutex.rdlock();
        m_locked=true;
    }
    ~ReadScopedLockImpl(){
        unLock();
    }

    void Lock(){
        if(!m_locked){
            m_mutex.rdlock();
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
    T& m_mutex;
    bool m_locked=false;
};

// 写锁
template<typename T>
class WriteScopedLockImpl{
public:
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex){
        m_mutex.wrlock();
        m_locked=true;
    }
    ~WriteScopedLockImpl(){
        unLock();
    }

    void Lock(){
        if(!m_locked){
            m_mutex.wrlock();
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
    T& m_mutex;     // 细节：这里不是引用类型的话就无法构造初始化
    bool m_locked=false;
};



// 互斥量
class MutexLock{
public:
    typedef ScopedLockImpl<MutexLock> scoped_lock;

    MutexLock(){
        pthread_mutex_init(&m_mutex,nullptr);
    }
    ~MutexLock(){
        pthread_mutex_destroy(&m_mutex);
    }

    // 上锁
    void lock(){
        pthread_mutex_lock(&m_mutex);
    }
    // 释放锁
    void unlock(){
        pthread_mutex_unlock(&m_mutex); 
    }

private:
    MutexLock(const MutexLock&)=delete;
    MutexLock(const MutexLock&&)=delete;
    MutexLock& operator=(const MutexLock&)=delete;
private:
    pthread_mutex_t m_mutex;
};

// 读写互斥量
class RWmutex{
public:
    // 局部读锁类
    typedef ReadScopedLockImpl<RWmutex> read_lock;
    // 局部写锁类
    typedef WriteScopedLockImpl<RWmutex> write_lock;

    RWmutex(){
        pthread_rwlock_init(&m_rwlock,nullptr);
    }
    ~RWmutex(){
        pthread_rwlock_destroy(&m_rwlock);
    }

    // 上读锁
    void wrlock(){
        pthread_rwlock_wrlock(&m_rwlock);
    }
    // 上写锁
    void rdlock(){
        pthread_rwlock_rdlock(&m_rwlock);
    }
    // 释放锁
    void unlock(){
        pthread_rwlock_unlock(&m_rwlock); 
    }

private:
    RWmutex(const RWmutex&)=delete;
    RWmutex(const RWmutex&&)=delete;
    RWmutex& operator=(const RWmutex&)=delete;
private:
    pthread_rwlock_t m_rwlock;
};

// 自旋锁
class SpinLock{
public:
    typedef ScopedLockImpl<SpinLock> scoped_lock;

    SpinLock(){
        pthread_spin_init(&m_spinlock,0);
    }
    ~SpinLock(){
        pthread_spin_destroy(&m_spinlock);
    }

    // 上锁
    void lock(){
        pthread_spin_lock(&m_spinlock);
    }
    // 释放锁
    void unlock(){
        pthread_spin_unlock(&m_spinlock); 
    }

private:
    SpinLock(const SpinLock&)=delete;
    SpinLock(const SpinLock&&)=delete;
    SpinLock& operator=(const SpinLock&)=delete;
private:
    pthread_spinlock_t m_spinlock;
};

// 原子锁
class AtomicLock{
public:
    typedef ScopedLockImpl<AtomicLock> scoped_lock;

    AtomicLock(){}
    ~AtomicLock(){}

    // 上锁
    void lock(){
        m_atomiclock.test_and_set();
    }
    // 释放锁
    void unlock(){
        m_atomiclock.clear();
    }

private:
    AtomicLock(const AtomicLock&)=delete;
    AtomicLock(const AtomicLock&&)=delete;
    AtomicLock& operator=(const AtomicLock&)=delete;
private:
    std::atomic_flag m_atomiclock=ATOMIC_FLAG_INIT;
};

}
#endif
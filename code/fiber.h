#ifndef __MYWEB_FIBER_H__
#define __MYWEB_FIBER_H__

#include <ucontext.h>
#include <memory>
#include <functional>
#include <string>
#include "mthread.h"

namespace myWeb{

class Fiber: public std::enable_shared_from_this<Fiber>{
public:
    typedef std::shared_ptr<Fiber> ptr;
    // 协程状态
    enum State{
        INIT,   // 创建
        HOLD,   // 挂起
        EXEC,   // 运行
        TERM,   // 终止
        READY,  // 就绪
        EXCEPT  // 异常
    };

private:
    // 设为私有，只允许类内部成员函数调用；用于构造mainFiber
    Fiber();
public:
    Fiber(std::function<void()> cb,size_t stacksize=0);
    ~Fiber();

    // 重置协程绑定的执行函数(减少协程创建和销毁时内存的创建和释放)
    void reset(std::function<void()> cb);
    // 将当前操作的协程切换到运行状态
    void swapIn();
    // 将当前操作的协程切换到后台
    void swapOut();
    // 获取ID
    const uint64_t& getID() const {
        return m_id;
    }
    // 获取当前状态
    State getState() const {
        return m_state;
    }

    // 设置当前运行的协程 
    static  void setThis(Fiber* fb);
    // 获取当前运行的协程指针(没有协程就创建主协程)
    static Fiber::ptr getThis();
    // 获取当前协程ID
    static uint64_t getThisFiberID();
    // 将当前执行的协程切换到READY状态
    static void yieldToReady();
    // 将当前执行的协程切换到HOLD状态
    static void yieldToHold();
    // 总协程数
    static uint64_t FibersInTotal();
    // 协程执行函数，执行完成返回到主协程上
    static void MainFunc();

private:
    // 协程ID
    uint64_t m_id=0;
    // 协程栈大小
    uint32_t m_stacksize;
    // 协程状态
    State m_state=INIT;
    // 协程上下文
    ucontext_t m_context;
    // 协程运行栈指针
    void* m_stk=nullptr;
    // 运行函数
    std::function<void()> m_cb;
};


}
#endif
#include <atomic>
#include "config.h"
#include "macro.h"
#include "fiber.h"
#include "log.h"

namespace myWeb{

// 当前协程ID
static std::atomic<uint64_t> s_fiber_id{0};
// 当前协程计数
static std::atomic<uint64_t> s_fiber_count{0};
// 当前协程指针
static thread_local Fiber* t_this_fiber=nullptr;
// 当前线程的主协程指针
static thread_local Fiber::ptr t_main_fiber=nullptr;

// 将栈大小添加至配置
ConfigVar<uint32_t>::ptr fiber_config=Config::Lookup("fiber.stack_size",uint32_t(1024*1024),"Fiber stack");

struct FiberConfigInit{
public:
    FiberConfigInit(){
        fiber_config->addListener([](const uint32_t& oldval,const uint32_t& newval){

        });
    }
};
FiberConfigInit __fiberInit;

// Malloc内存分配器
class MallocStackAllocator{
public:
    static void* mallocAlloc(size_t size){
        return malloc(size);
    }
    static void freeAlloc(void* vp,size_t size){
        // 为兼容 unmap()需要size参数
        return free(vp);
    }
};

// 在这里更换内存分配器
typedef MallocStackAllocator StackAllocator;

void Fiber::setThis(Fiber* fb){
    t_this_fiber=fb;
}

Fiber::ptr Fiber::getThis(){
    if(t_this_fiber){       
        return t_this_fiber->shared_from_this();
    }
    // 如果为nullptr，则没有协程，需从主协程开始创建
    Fiber::ptr mainFiber(new Fiber());
    MYWEB_ASSERT(mainFiber.get()==t_this_fiber);    // 判断是否构造成功
    t_main_fiber=mainFiber;
    return t_this_fiber->shared_from_this();
}

uint64_t Fiber::getThisFiberID(){
    
}
void Fiber::yieldToReady(){

}
void Fiber::yieldToHold(){

}
void Fiber::FibersInTotal(){

}
void Fiber::MainFunc(){

}

Fiber::Fiber(){
    m_state=EXEC;
    setThis(this);
    if(getcontext(&m_context)){
        MYWEB_ASSERT_2(false,"getcontext");
    }
    ++s_fiber_count;
    INLOG_DEBUG(MYWEB_NAMED_LOG("system"))<<"Main Fiber Established";
}

Fiber::Fiber(std::function<void()> cb,size_t stacksize)
        :m_id(++s_fiber_id)
        ,m_cb(cb){
    ++s_fiber_count;
    // 设置协程栈
    m_stacksize=stacksize?stacksize:fiber_config->getVal();     // 初始化栈大小
    m_stk=StackAllocator::mallocAlloc(m_stacksize);          // 初始化栈指针   
    if(getcontext(&m_context)){
        MYWEB_ASSERT_2(false,"getcontext");
    }
    m_context.uc_link=nullptr;              // 关联的上下文
    m_context.uc_stack.ss_sp=m_stk;         // 保存关联上下文的栈指针
    m_context.uc_stack.ss_size=m_stacksize; // 保存关联上下文的栈大小

    makecontext(&m_context,&MainFunc,0);
    INLOG_DEBUG(MYWEB_NAMED_LOG("system"))<<"Fiber ID: "<<m_id;
}

Fiber::~Fiber(){
    --s_fiber_count;
    if(m_stk){      // 子协程
        StackAllocator::freeAlloc(m_stk,m_stacksize);
        if(m_state!=TERM || m_state!=EXCEPT || m_state!=INIT){
            
            MYWEB_ASSERT_2(false,"Fiber Destruct Error")
        }
    }else{          // 主协程
        MYWEB_ASSERT(!m_cb);
        MYWEB_ASSERT(m_state==EXEC);
        Fiber* cur=t_this_fiber;
        if(cur==this){
            setThis(nullptr);
        }
    }
}

void Fiber::reset(std::function<void()> cb){
    MYWEB_ASSERT_2(m_stk,"Main Fiber Reset");
    MYWEB_ASSERT_2(m_state==TERM || m_state==INIT,"Wrong State Reset");
    m_cb=cb;
    if(getcontext(&m_context)){
        MYWEB_ASSERT_2(false,"getcontext");
    }
    m_context.uc_link=nullptr;              // 关联的上下文
    m_context.uc_stack.ss_sp=m_stk;         // 保存关联上下文的栈指针
    m_context.uc_stack.ss_size=m_stacksize; // 保存关联上下文的栈大小

    makecontext(&m_context,&MainFunc,0);
    m_state=INIT;
}

void Fiber::swapIn(){

}

void Fiber::swapOut(){

}




}
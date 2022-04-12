#ifndef __MYWEB_THREAD_H__
#define __MYWEB_THREAD_H__

#include <thread>
#include <functional>
#include <memory>

namespace myWeb{

class Thread{
public:
    typedef std::shared_ptr<Thread> ptr;
    
private:
    Thread(const Thread&)=delete;       // Thread类不可被拷贝
    Thread(const Thread&&)=delete;
    Thread& operator=(const Thread&)=delete;

private:

};

}

#endif
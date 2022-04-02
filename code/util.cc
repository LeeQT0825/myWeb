#include "util.h"

namespace myWeb{

pid_t GetThreadID(){
    // 1. gettid() ---系统调用(TODO:开销较大)，可以用宏"__NR_gettid"替代，获取的是内核中真实的线程ID
    // （linux内核其实没有线程的概念，所以内核线程就是“内核进程”，所以返回的是pid_t，当用户进程中只有一个线程的时候返回的是该进程号
    // 2. pthread_self() 获取的是POSIX描述的线程ID（是相对于进程的线程控制块的首地址
    // 3. std::thread::get_id() 获取的是用户线程ID
    return syscall(__NR_gettid);       
}

int32_t GetFiberID(){
    return 0;
}
}
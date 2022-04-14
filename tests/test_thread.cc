#include "../code/myweb.h"
#include <vector>
#include <iostream>
#include <time.h>

int count=0;
// myWeb::RWmutex rwmtx;    // 读写锁
myWeb::MutexLock mtx;

void func2();
void func3();

void func1(){
    INLOG_INFO(MYWEB_ROOT_LOG)<<"func1: "<<'\n'
                              <<"Thread id: "<<myWeb::GetThreadID()<<'\n'
                              <<"Thread name: "<<myWeb::Thread::getThisThreadName()<<'\n'
                              <<"id: "<<myWeb::Thread::getThisThread()->getID()<<'\n'
                              <<"name: "<<myWeb::Thread::getThisThread()->getName();
                            //   <<"pthreadID: "<<myWeb::Thread::getThisThread()->get_pthreadID();
    // sleep(1);
    // func2();

    for(int i=0;i<100000;++i){
        // myWeb::RWmutex::WriteLock wlock(rwmtx);      // 写锁，结果正确，0.001881s
        // myWeb::RWmutex::ReadLock rlock(rwmtx);      // 读锁，结果错误
        myWeb::MutexLock::mutex_lock mlock(mtx);        // 互斥锁，结果正确，0.002757s
        ++count;
    }
}

void func2(){
    INLOG_INFO(MYWEB_ROOT_LOG)<<myWeb::Thread::getThisThreadName()<<" in func2";
    std::cout<<"-----------------------------------------------"<<std::endl;
}


int main(){
    std::vector<myWeb::Thread::ptr> thrds;
    std::clock_t start,end;
    start=clock();
    for(int i=0;i<5;++i){
        myWeb::Thread::ptr th(new myWeb::Thread(&func1,"name_"+std::to_string(i)));
        thrds.push_back(th);
    }
    end=clock();


    for(auto& i:thrds){
        i->join();
    }

    std::cout<<"test finished"<<std::endl;
    std::cout<<"count: "<<count<<" time: "<<double(end-start)/CLOCKS_PER_SEC<<std::endl;

    return 0;
}
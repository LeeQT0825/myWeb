#include "../code/myweb.h"
#include <vector>
#include <iostream>
#include <chrono>

int count=0;
myWeb::RWmutex rwmtx;    // 读写锁
// myWeb::MutexLock mtx;

void func2();

void func1(){
    std::cout<<"+++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
    INLOG_INFO(MYWEB_ROOT_LOG)<<"func1: "<<'\n'
                              <<"Thread id: "<<myWeb::GetThreadID()<<'\n'
                              <<"Thread name: "<<myWeb::Thread::getThisThreadName()<<'\n'
                              <<"id: "<<myWeb::Thread::getThisThread()->getID()<<'\n'
                              <<"name: "<<myWeb::Thread::getThisThread()->getName();
                            //   <<"pthreadID: "<<myWeb::Thread::getThisThread()->get_pthreadID();
    std::cout<<"+++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
    // sleep(1);
    // func2();

    // for(int i=0;i<100000;++i){
    //     // myWeb::RWmutex::WriteLock wlock(rwmtx);      // 写锁，结果正确，0.001881s
    //     // myWeb::RWmutex::ReadLock rlock(rwmtx);      // 读锁，结果错误
    //     myWeb::MutexLock::mutex_lock mlock(mtx);        // 互斥锁，结果正确，0.002757s
    //     ++count;
    // }
}

void func2(){
    for(int i=0;i<10000;++i){
        // myWeb::RWmutex::write_lock rlock(rwmtx);
        INLOG_INFO(MYWEB_NAMED_LOG("mutex"))<<"====================================";
    }
}


int main(){
    std::vector<myWeb::Thread::ptr> thrds;
    myWeb::Config::LoadFromYaml("/home/lee/projects/VScode/myProject/myconfig.yml");
    std::cout<<myWeb::Config::Lookup("logs")->ToString()<<std::endl;

    auto begin = std::chrono::high_resolution_clock::now();
    for(int i=0;i<5;++i){
        myWeb::Thread::ptr th(new myWeb::Thread(&func1,"name_"+std::to_string(i)));
        thrds.push_back(th);
    }

    for(auto& i:thrds){
        i->join();
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::cout<<"-----------------------------------------------"<<std::endl;
    INLOG_INFO(MYWEB_ROOT_LOG)<<"test finished";
    INLOG_INFO(MYWEB_NAMED_LOG("test"))
            <<"count: "<<count
            <<" time: "<<std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()<<"ns";

    return 0;
}
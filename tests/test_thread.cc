#include "../code/myweb.h"
#include <vector>
#include <iostream>

void func2();
void func3();

void func1(){
    INLOG_INFO(MYWEB_ROOT_LOG)<<"func1: "<<'\n'
                              <<"Thread id: "<<myWeb::GetThreadID()<<'\n'
                              <<"Thread name: "<<myWeb::Thread::getThisThreadName()<<'\n'
                              <<"id: "<<myWeb::Thread::getThisThread()->getID()<<'\n'
                              <<"name: "<<myWeb::Thread::getThisThread()->getName();
                            //   <<"pthreadID: "<<myWeb::Thread::getThisThread()->get_pthreadID();
    sleep(1);
    func2();
    sleep(2);
    func3();
}

void func2(){
    INLOG_INFO(MYWEB_ROOT_LOG)<<myWeb::Thread::getThisThreadName()<<" in func2";
    std::cout<<"-----------------------------------------------"<<std::endl;
}

void func3(){
    INLOG_INFO(MYWEB_ROOT_LOG)<<myWeb::Thread::getThisThreadName()<<" in func3";
    std::cout<<"-----------------------------------------------"<<std::endl;
}

int main(){
    std::vector<myWeb::Thread::ptr> thrds;
    for(int i=0;i<5;++i){
        myWeb::Thread::ptr th(new myWeb::Thread(&func1,"name_"+std::to_string(i)));
        thrds.push_back(th);
    }

    for(auto& i:thrds){
        i->join();
    }

    std::cout<<"test finished"<<std::endl;

    return 0;
}
#include "../code/myweb.h"
#include <vector>

void func1(){
    INLOG_INFO(MYWEB_ROOT_LOG)<<"Thread id: "<<myWeb::Thread::getThisThread()->getID()<<'\n'
                              <<"Thread name: "<<myWeb::Thread::getThisThreadName()
}

void func2(){}

int main(){
    std::vector<myWeb::Thread::ptr> thrds;
    for(int i=0;i<5;++i){
        myWeb::Thread::ptr th(new myWeb::Thread(&func1,"name_"+std::to_string(i)));
        thrds.push_back(th);
    }



    return 0;
}
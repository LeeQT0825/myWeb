#include "../code/myweb.h"
#include <iostream>

void fb_func1();
void fb_func2();

void th_func1(){
    myWeb::Fiber::ptr mFiber_th1=myWeb::Fiber::getThis();

    myWeb::Fiber::ptr fb1(new myWeb::Fiber(fb_func1));
    myWeb::Fiber::ptr fb2(new myWeb::Fiber(fb_func2));
    fb1->call();
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"fb1 swapout";
    fb2->call();
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"fb2 swapout";
    fb1->call();
    fb2->call();
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"fb finished";
}

void fb_func1(){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"run fb_fun1 "<<"fiberID: "<<myWeb::Fiber::getThisFiberID();
    myWeb::Fiber::getThis()->swapOut();
    // myWeb::Fiber::getThis()->yieldToHold();
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"fb1 back";
}

void fb_func2(){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"run fb_fun2 "<<"fiberID: "<<myWeb::Fiber::getThisFiberID();
    myWeb::Fiber::getThis()->swapOut();
    // myWeb::Fiber::getThis()->yieldToReady();
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"fb2 back";
}

int main(){
    LOADYAML;

    myWeb::Thread::ptr th1(new myWeb::Thread(th_func1,"thread_01"));
    th1->join();
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"test finished.";
    return 0;
}
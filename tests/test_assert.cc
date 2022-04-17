#include "../code/myweb.h"
#include <fstream>

void test1();
void test2();
int a=0;
void test(){
    test1();
}

void test1(){
    LOADYAML;
    a++;
    test2();
}

void test2(){
    // INLOG_INFO(MYWEB_ROOT_LOG)<<myWeb::BacktraceToString(5);
    // INLOG_INFO(MYWEB_ROOT_LOG)<<myWeb::Config::Lookup("logs")->ToString();
    // INLOG_ERROR(MYWEB_NAMED_LOG("mutex"))<<"hello";      // 也不行
    std::ofstream fd;
    fd.open("/home/lee/projects/VScode/myProject/log/sys_log.txt",std::ios_base::app);
    fd<<"hello~";
    fd.close();         // 依然没有输出
    MYWEB_ASSERT(a==0); 
    // if(!(a==0)){
    //     INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"ASSERTION: " 
    //             <<myWeb::BacktraceToString(5); 
    //     // assert(a==0); 
    // }
}

int main(){
    LOADYAML;
    std::ofstream fd;
    fd.open("/home/lee/projects/VScode/myProject/log/sys_log.txt",std::ios_base::app);
    fd<<"hello~";
    fd.close();         // 依然没有输出
    // INLOG_INFO(MYWEB_ROOT_LOG)<<myWeb::Config::Lookup("logs")->ToString();
    // myWeb::Config::LoadFromYaml("/home/lee/projects/VScode/myProject/myconfig.yml");
    test();
    return 0;
}
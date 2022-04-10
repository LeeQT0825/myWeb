#include <iostream>
#include "../code/config.h"
#include "../code/log.h"

myWeb::ConfigVar<int>::ptr testVar1=myWeb::Config::Lookup<int>("system.port",8080,"system port");
myWeb::ConfigVar<float>::ptr testVar2=myWeb::Config::Lookup<float>("system.value",15.4f,"system value");

void test_yaml(){
    YAML::Node yml_node=YAML::LoadFile("../log.yml");       // 这里最好用绝对路径 
    INLOG_INFO(MYWEB_ROOT_LOG)<<yml_node;
}

// 测试载入配置文件
void test_config(){
    INLOG_INFO(MYWEB_ROOT_LOG)<< "before: " <<testVar1->getName()<<" "<<testVar1->getVal();
    INLOG_INFO(MYWEB_ROOT_LOG)<< "before: " <<testVar2->getName()<<" "<<testVar2->getVal();
    myWeb::Config::LoadFromYaml("../log.yml");
    INLOG_INFO(MYWEB_ROOT_LOG)<< "after: " <<testVar1->getName()<<" "<<testVar1->getVal();
    INLOG_INFO(MYWEB_ROOT_LOG)<< "after: " <<testVar2->getName()<<" "<<testVar2->ToString();
}

// 测试变更回调函数
void test_cb(){
    INLOG_INFO(MYWEB_ROOT_LOG)<< "before: " <<testVar1->getName()<<" "<<testVar1->getVal();

    testVar1->addListener([](const int& oldval,const int& newval){
        INLOG_INFO(MYWEB_ROOT_LOG)<<"old: "<<oldval<<" new: "<<newval;
    });
    myWeb::Config::LoadFromYaml("../log.yml");

    INLOG_INFO(MYWEB_ROOT_LOG)<< "after: " <<testVar1->getName()<<" "<<testVar1->getVal();
}

// 测试日志系统的配置变量及回调函数
void test_log_cb(){
    // 默认rootlog:
    INLOG_INFO(MYWEB_ROOT_LOG)<< "before: " <<"level: "<<myWeb::LogLevel::ToString(MYWEB_ROOT_LOG->getlevel());
    INLOG_DEBUG(MYWEB_NAMED_LOG("system"))<< "before: " <<"level: "<<myWeb::LogLevel::ToString(MYWEB_NAMED_LOG("system")->getlevel());
    myWeb::Config::LoadFromYaml("../log.yml");
    INLOG_INFO(MYWEB_ROOT_LOG)<< "after: " <<"level: "<<myWeb::LogLevel::ToString(MYWEB_ROOT_LOG->getlevel());
    INLOG_DEBUG(MYWEB_NAMED_LOG("system"))<< "after: " <<"level: "<<myWeb::LogLevel::ToString(MYWEB_NAMED_LOG("system")->getlevel());
    INLOG_WARN(MYWEB_NAMED_LOG("system"))<< "after: " <<"level: "<<myWeb::LogLevel::ToString(MYWEB_NAMED_LOG("system")->getlevel())<<" infile.";

}

int main(){
    // MYWEB_ROOT_LOG->setlevel(myWeb::LogLevel::INFO);
    // test_yaml();    
    // test_config();
    // test_cb();
    test_log_cb();
    return 0;
}


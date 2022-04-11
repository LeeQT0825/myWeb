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
    std::cout<<myWeb::logMgr::getInstance()->toYamlString()<<std::endl;
    std::cout<<"---------------------------------------------"<<std::endl;
    myWeb::Config::LoadFromYaml("/home/lee/projects/VScode/myProject/log.yml");
    std::cout<<myWeb::Config::Lookup("logs")->ToString()<<std::endl;
    std::cout<<"---------------------------------------------"<<std::endl;
    myWeb::Config::LoadFromYaml("/home/lee/projects/VScode/myProject/log2.yml"); 
    std::cout<<myWeb::Config::Lookup("logs")->ToString()<<std::endl;

}

int main(){
    // MYWEB_ROOT_LOG->setlevel(myWeb::LogLevel::INFO);
    // test_yaml();    
    // test_config();
    // test_cb();
    test_log_cb();
    return 0;
}


#include <iostream>
#include "../code/config.h"
#include "../code/log.h"

myWeb::ConfigVar<int>::ptr testVar1=myWeb::Config::Lookup<int>("system.port",8080,"system port");
myWeb::ConfigVar<float>::ptr testVar2=myWeb::Config::Lookup<float>("system.value",15.4f,"system value");

void test_yaml(){
    YAML::Node yml_node=YAML::LoadFile("../log.yml");
    INLOG_INFO(MYWEB_ROOT_LOG)<<yml_node;
}

void test_config(){
    INLOG_INFO(MYWEB_ROOT_LOG)<< "before: " <<testVar1->getName()<<" "<<testVar1->getVal();
    INLOG_INFO(MYWEB_ROOT_LOG)<< "before: " <<testVar2->getName()<<" "<<testVar2->getVal();
    myWeb::Config::LoadFromYaml("../log.yml");
    INLOG_INFO(MYWEB_ROOT_LOG)<< "after: " <<testVar1->getName()<<" "<<testVar1->getVal();
    INLOG_INFO(MYWEB_ROOT_LOG)<< "after: " <<testVar2->getName()<<" "<<testVar2->ToString();
}

void test_cb(){
    INLOG_INFO(MYWEB_ROOT_LOG)<< "before: " <<testVar1->getName()<<" "<<testVar1->getVal();

    testVar1->addListener(1,[](const int& oldval,const int& newval){
        INLOG_INFO(MYWEB_ROOT_LOG)<<"old: "<<oldval<<" new: "<<newval;
    });
    myWeb::Config::LoadFromYaml("../log.yml");

    INLOG_INFO(MYWEB_ROOT_LOG)<< "after: " <<testVar1->getName()<<" "<<testVar1->getVal();
}

int main(){
    MYWEB_ROOT_LOG->setlevel(myWeb::LogLevel::INFO);
    // test_yaml();    
    // test_config();
    test_cb();
    return 0;
}


#include <iostream>
#include "../code/config.h"
#include "../code/log.h"

myWeb::ConfigVar<int>::ptr testVar=myWeb::Config::Lookup<int>("system_port",8080,"system port");

int main(){
    MYWEB_ROOT_LOG->setlevel(myWeb::LogLevel::INFO);
    INLOG_INFO(MYWEB_ROOT_LOG)<<testVar->getName()<<" "<<testVar->getVal()<<" "<<testVar->getDiscription();

    return 0;
}


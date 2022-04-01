#include <iostream>
#include "../code/log.h"

int main(){
    myWeb::Logger::ptr logger(new myWeb::Logger("test"));
    logger->addappender(myWeb::LogAppender::ptr(new myWeb::StdoutLogAppender));

    myWeb::LogEvent::ptr event(new myWeb::LogEvent(logger,"myfile",myWeb::LogLevel::DEBUG,101,"threadName",1,2,time(0),4));
    logger->log(myWeb::LogLevel::DEBUG,event);


    return 0;
} 
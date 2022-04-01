#include <iostream>
#include "../code/log.h"

int main(){
    myWeb::Logger::ptr logger(new myWeb::Logger("test"));
    logger->addappender(myWeb::LogAppender::ptr(new myWeb::StdoutLogAppender));

    myWeb::LogEvent::ptr event(new myWeb::LogEvent(logger,__FILE__,myWeb::LogLevel::DEBUG,__LINE__,"threadName",1,2,time(NULL),4));
    logger->log(myWeb::LogLevel::DEBUG,event);

    return 0;
} 
#include <iostream>
#include <thread>
#include "../code/log.h"
#include "../code/util.h"

int main(){
    myWeb::Logger::ptr logger(new myWeb::Logger("test"));
    logger->addappender(myWeb::LogAppender::ptr(new myWeb::StdoutLogAppender));

    // myWeb::LogEvent::ptr event(new myWeb::LogEvent(logger,__FILE__,myWeb::LogLevel::DEBUG,__LINE__,"threadName",myWeb::GetThreadID(),myWeb::GetFiberID(),time(NULL),4));
    // logger->log(myWeb::LogLevel::WARN,event);
    
    INLOG_DEBUG(logger) << "log test";
    INLOG_FATAL(logger)<<"test fatal";

    return 0;
} 
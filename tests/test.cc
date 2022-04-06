#include <iostream>
#include <thread>
#include "../code/log.h"
#include "../code/util.h"

int main(){
    // myWeb::Logger::ptr logger(new myWeb::Logger("test"));
    // myWeb::LogAppender::ptr fileAppender(new myWeb::FileLogAppender("log/log.txt"));
    // myWeb::LogAppender::ptr StdoutAppender(new myWeb::StdoutLogAppender);

    // fileAppender->setAppenderLevel(myWeb::LogLevel::DEBUG);
    // fileAppender->setFormatter(myWeb::LogFomatter::ptr(new myWeb::LogFomatter("%d{%Y-%m-%d %H:%M:%S} [%p]%T<%f:%l> %m %n")));
    // StdoutAppender->setAppenderLevel(myWeb::LogLevel::WARN);
    // logger->addappender(fileAppender);
    // logger->addappender(StdoutAppender);
    
    // myWeb::LogEvent::ptr event(new myWeb::LogEvent(logger,__FILE__,myWeb::LogLevel::DEBUG,__LINE__,"threadName",myWeb::GetThreadID(),myWeb::GetFiberID(),time(NULL),4));
    // logger->log(myWeb::LogLevel::WARN,event);
    
    myWeb::Logger::ptr logger=MYWEB_ROOT_LOG;
    INLOG_DEBUG(logger) << "log test";
    INLOG_FATAL(logger)<<"test fatal";

    return 0;
} 
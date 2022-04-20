#ifndef __MYWEB_LOG_H__
#define __MYWEB_LOG_H__

#include <vector>
#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <fstream>
#include <sstream>
#include <iostream>
#include <time.h>
#include <unordered_map>
#include "util.h"
#include "singleton.h"
#include "mmutex.h"

#define INLOG_LEVEL(logger,level) \
    if(level>=logger->getlevel())\
        myWeb::LogEventWrap(myWeb::LogEvent::ptr(new myWeb::LogEvent(logger,__FILE__,\
        level,__LINE__,"test",myWeb::GetThreadID(),\
        myWeb::GetFiberID(),time(NULL),0))).GetSS()
// 使用日志器的一般接口（需自定义 Logger）
#define INLOG_UNKNOW(logger) INLOG_LEVEL(logger,myWeb::LogLevel::UNKNOW)
#define INLOG_DEBUG(logger) INLOG_LEVEL(logger,myWeb::LogLevel::DEBUG)
#define INLOG_INFO(logger) INLOG_LEVEL(logger,myWeb::LogLevel::INFO)
#define INLOG_WARN(logger) INLOG_LEVEL(logger,myWeb::LogLevel::WARN)
#define INLOG_ERROR(logger) INLOG_LEVEL(logger,myWeb::LogLevel::ERROR)
#define INLOG_FATAL(logger) INLOG_LEVEL(logger,myWeb::LogLevel::FATAL)

// 获取全局主日志器
#define MYWEB_ROOT_LOG myWeb::logMgr::getInstance()->getRoot()
// 获取指定日志器
#define MYWEB_NAMED_LOG(name) myWeb::logMgr::getInstance()->getLogger(name)

namespace myWeb{

class Logger;

// log等级 
class LogLevel{
public:
    enum Level{
        UNKNOW=0,
        DEBUG=1,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    static const char* ToString(LogLevel::Level level);     //静态类成员函数: 无需实例化，其他类可以直接访问
    static Level FromString(const std::string& str){

        #define XX(level,s) if(str==#s){\
            return LogLevel::level;\
        }
        XX(UNKNOW,unknow);
        XX(DEBUG,debug);
        XX(INFO,info);
        XX(WARN,warn);
        XX(ERROR,error);
        XX(FATAL,fatal);
        XX(UNKNOW,UNKNOW);
        XX(DEBUG,DEBUG);
        XX(INFO,INFO);
        XX(WARN,WARN);
        XX(ERROR,ERROR);
        XX(FATAL,FATAL);
        #undef XX
        return UNKNOW;
    }
};


// 日志事件存储器
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger,const char* file,LogLevel::Level level,
            int32_t line,const std::string& threadName,int32_t threadID,
            uint64_t fiberID,uint64_t time,uint64_t elapse)
        :m_file(file),
        m_line(line),
        m_threadID(threadID),
        m_Thread_Name(threadName),
        m_fiberID(fiberID),
        m_time(time),
        m_elapse(elapse),
        m_level(level),
        m_logger(logger){}

    const char* getFile() const{
        return m_file;
    }

    LogLevel::Level getLevel() const{
        return m_level;
    }

    int32_t getLine() const{
        return m_line;
    }

    int32_t getThreadID() const{
        return m_threadID;
    }

    const std::string& getTreadName() const{
        return m_Thread_Name;
    }

    uint64_t getFiberID() const{
        return m_fiberID;
    }

    uint64_t getTime() const{
        return m_time;
    }

    uint64_t getElapse() const{
        return m_elapse;
    }

    std::stringstream& getSS(){
        return m_ss;
    }

    std::string getContent(){         // 这里有左右值引用的细节
        return m_ss.str();
    }

    std::shared_ptr<Logger> getLogger(){
        return m_logger;
    }

private:
    // 文件名
    const char* m_file=nullptr;
    // 行号
    int32_t m_line=0; 
    // 线程号
    int32_t m_threadID;     // std::thread::id 重载了<<，可以输出到流
    // 线程名称
    std::string m_Thread_Name;
    // 协程号
    uint64_t m_fiberID=0;
    // 时间戳
    uint64_t m_time;
    // 程序启动到现在的毫秒数
    uint64_t m_elapse;
    // 日志等级
    LogLevel::Level m_level;
    // 日志内容
    std::stringstream m_ss;
    // 日志器
    std::shared_ptr<Logger> m_logger;
    // Logger::ptr m_logger;        // typedef 作用在编译阶段，从它定义的地方以后才可以使用，所以这里不能这么用
    
};

// 封装LogEvent智能指针，方便Logger::log()调用
class LogEventWrap{
public:
    LogEventWrap(LogEvent::ptr event);
    ~LogEventWrap();

    std::stringstream& GetSS(){
        return m_event->getSS();
    }
private:
    LogEvent::ptr m_event;
};

// 日志格式器: 可以解析用户传入的模板
class LogFomatter{
public:
    typedef std::shared_ptr<LogFomatter> ptr;

    LogFomatter(const std::string& patten):m_patten(patten){
        init();
    }
    // 返回格式化日志文本（重载）
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event);

    std::string getPatten() const;

public:
    // 初始化，解析日志模板 
    void init();
    bool isError(){ return m_error;}

    // 日志内容项格式化
    class FormatItem{       
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        FormatItem(const std::string& fmt = ""){}   // 这里需要传入格式内容的原因是“统一接口”，比如DateTimeFormatItem子类就需要它
        virtual ~FormatItem(){}
        // 格式化日志到标准流
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)=0;   //纯虚函数

    };

private:
    std::string m_patten;                        // 日志格式模板
    std::vector<FormatItem::ptr> m_items;        // 内容项解析后暂存
    bool m_error=false;
};



// 日志输出落地(包含输出格式)
class LogAppender{
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef SpinLock lock_type;     // 方便统一更换锁

    virtual ~LogAppender(){}
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event)=0;      // 子类必须实现

    void setFormatter(LogFomatter::ptr val){
        m_formatter=val;
        m_hasformatter=true;
    }
    LogFomatter::ptr getFormatter() const { return m_formatter;}

    void setAppenderLevel(LogLevel::Level level){m_level=level;}
    LogLevel::Level getAppenderLevel() const {return m_level;}

    virtual std::string toYamlString() const =0;

protected:      // 子类要用到的
    LogLevel::Level m_level=LogLevel::DEBUG;    // 默认
    LogFomatter::ptr m_formatter;
    bool m_hasformatter=false;
    RWmutex m_rw_mutex;
    lock_type m_lock;
};


// 输出到控制台
class StdoutLogAppender:public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr; 

    void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;

    std::string toYamlString() const override;
};

// 输出到文件
class FileLogAppender:public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& filename):m_files(filename){
        reopen();
    }
    void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;   // override 证明确实是从父类继承来的重载的实现
    // 重新打开文件，打开成功返回true
    bool reopen();  

    std::string toYamlString() const override;

private:
    std::string m_files;        //唯一性
    std::ofstream m_filestream;
};

// 日志器：控制日志输出级别、及是否输出
class Logger:public std::enable_shared_from_this<Logger>{
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef SpinLock lock_type;

    Logger(const std::string& name="root")
    :m_name(name),
    m_level(LogLevel::DEBUG){
        m_formatter.reset(new LogFomatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]<%f:%l> %m %n"));       // 定义一个默认的日志格式
    }
    void log(LogLevel::Level level,LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addappender(LogAppender::ptr appender);
    void delappender(LogAppender::ptr appender);
    void clearAppenders();

    void setFormatter(LogFomatter::ptr formatter);
    void setFormatter(const std::string& str);

    const std::string& getName() const {
        return m_name;
    }
    LogLevel::Level getlevel() const {
        return m_level;
    }
    void setlevel(LogLevel::Level val){
        m_level=val;
    }

    std::string toYamlString() const;

private: 
    std::string m_name;                             //日志名称
    LogLevel::Level m_level=LogLevel::UNKNOW;       //日志级别限制
    std::list<LogAppender::ptr> m_appenders;        //路径列表(基类的指针)
    LogFomatter::ptr m_formatter;                   //格式器：默认格式器可分派给所有的appender
    lock_type m_lock;
};

// 应该是全局单例的
// 封装多个Logger，提供默认构造的logger，创建宏，方便一步调用
class LogManager{
public:
    typedef SpinLock lock_type;
    // 创建默认主日志器
    LogManager();

    // 获取日志器(没有就加上一个)
    Logger::ptr getLogger(const std::string& name);
    // 获取主日志器
    Logger::ptr getRoot() const {return m_root;}

    // 输出所有日志器配置
    std::string toYamlString() const ;
private:
    // 日志容器
    std::unordered_map<std::string,Logger::ptr> m_logMap;
    // 根据 LogManager 中的主日志器设置
    Logger::ptr m_root;
    lock_type m_lock;
};

typedef myWeb::Singleton<LogManager> logMgr;

}
#endif

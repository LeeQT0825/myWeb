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

    static const char* ToString(LogLevel::Level level);     //TODO 为什么是静态类成员函数: 无需实例化，其他类可以直接访问
};


// 日志事件存储器
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger,const char* file,LogLevel::Level level,
            int32_t line,const std::string& threadName,uint32_t threadID,
            uint32_t fiberID,uint64_t time,uint64_t elapse)
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

    uint32_t getThreadID() const{
        return m_threadID;
    }

    const std::string& getTreadName() const{
        return m_Thread_Name;
    }

    uint32_t getFiberID() const{
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
    uint32_t m_threadID=0;
    // 线程名称
    std::string m_Thread_Name;
    // 协程号
    uint32_t m_fiberID=0;
    // 时间戳
    uint64_t m_time;
    // 程序启动到现在的毫秒数
    uint64_t m_elapse;
    // 日志等级
    LogLevel::Level m_level;
    // log内容
    std::stringstream m_ss;
    // 日志器
    std::shared_ptr<Logger> m_logger;
    // Logger::ptr m_logger;        // typedef 作用在编译阶段，从它定义的地方以后才可以使用，所以这里不能这么用
    
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
        // 格式化日志到流
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

    virtual ~LogAppender(){}
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,LogEvent::ptr event)=0;      // 子类必须实现

    void setFormatter(LogFomatter::ptr val){
        m_formatter=val;
        m_hasformatter=true;
    }
    LogFomatter::ptr getFormatter() const { return m_formatter;}

protected:      // 子类要用到的
    LogLevel::Level m_level;
    LogFomatter::ptr m_formatter;
    bool m_hasformatter=false;
};


// 输出到控制台
class StdoutLogAppender:public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr; 

    void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;
};

// 输出到文件
class FileLogAppender:public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& filename):m_files(filename){}
    void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;   // override 证明确实是从父类继承来的重载的实现
    // 重新打开文件，打开成功返回true
    bool reopen();  
private:
    std::string m_files;        //唯一性
    std::ofstream m_filestream;
};

// 日志器：控制日志输出级别、及是否输出
class Logger:public std::enable_shared_from_this<Logger>{
public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name="root")
    :m_name(name),
    m_level(LogLevel::DEBUG){
        m_formatter.reset(new LogFomatter("%d [%p] %f %l %m %n"));       // 定义一个默认的日志格式
    }
    //
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

private: 
    std::string m_name;                             //日志名称
    LogLevel::Level m_level;                        //日志级别
    std::list<LogAppender::ptr> m_appenders;        //路径列表(基类的指针)
    LogFomatter::ptr m_formatter;                   //格式器：默认格式器可分派给所有的appender
    Logger::ptr m_root;                             //主日志器
};

}
#endif

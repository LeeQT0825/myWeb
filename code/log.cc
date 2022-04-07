#include "log.h"
#include <functional>
#include <string.h>
using namespace myWeb;

namespace myWeb{

const char* LogLevel::ToString(LogLevel::Level level){
    switch(level){

// 宏定义函数："\"是换行继续，#name 表示使用传入的“参数名称”字符串，并不是name本身
// 注意：！！换行符后不能有任何字符，注释也不行，空格也不行（用ctrl+右方向键检测）
    #define XX(name)\
        case LogLevel::name:\
            return #name;\
            break;  
         
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
    #undef XX   

        default:
            return "UNKNOW";
    }
    return "UNKNOW";
}


void Logger::log(LogLevel::Level level,LogEvent::ptr event){
    if(level>=m_level){                 // 级别大于标定级别则输出
        auto selfptr=shared_from_this();

        for(auto i:m_appenders){        // 输出到每个路径
            i->log(selfptr,level,event);
        }
    }
}

void Logger::debug(LogEvent::ptr event){
    log(LogLevel::DEBUG,event);
}
void Logger::info(LogEvent::ptr event){
    log(LogLevel::INFO,event);
}
void Logger::warn(LogEvent::ptr event){
    log(LogLevel::WARN,event);
}
void Logger::error(LogEvent::ptr event){
    log(LogLevel::ERROR,event);
}
void Logger::fatal(LogEvent:: ptr event){
    log(LogLevel::FATAL,event);
}

LogEventWrap::LogEventWrap(LogEvent::ptr event):m_event(event){}

LogEventWrap::~LogEventWrap(){
    m_event->getLogger()->log(m_event->getLevel(),m_event);
}

void Logger::addappender(LogAppender::ptr appender){
    if(!appender->getFormatter()){
        appender->setFormatter(m_formatter);    // 如果传入的appender没有formatter，就把自己的给它
    }
    m_appenders.push_back(appender);
}
void Logger::delappender(LogAppender::ptr appender){
    for(auto iter=m_appenders.begin();
            iter!=m_appenders.end();iter++){
        if(*iter==appender) {
            m_appenders.erase(iter);
            break;
        }
    }
}
void Logger::clearAppenders(){
    m_appenders.clear();
}

void Logger::setFormatter(LogFomatter::ptr formatter){
    m_formatter=formatter;
    for(auto i:m_appenders){
        if(!i->m_hasformatter){
            i->setFormatter(formatter);
        }
    }
}
void Logger::setFormatter(const std::string& str){
    LogFomatter::ptr newFormat=std::make_shared<LogFomatter>(LogFomatter(str));
    if(newFormat->isError()){
        std::cout<<"Logger setFormatter str= "<<m_name
                 <<" value= "<<str<<" is invalid formatter"<<std::endl;
        return;
    }
    setFormatter(newFormat);
}



void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    if(level>=m_level){
        std::cout<<m_formatter->format(logger,level,event);   
    }
}

bool FileLogAppender::reopen(){
    if(m_filestream){
        m_filestream.close();
    } 
    return myWeb::FileUtils::OpenForWrite(m_filestream,m_files,std::ios_base::app);

}

void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    if(level>=m_level){
        m_filestream<<m_formatter->format(logger,level,event);
    }  
}

// 解析子模块
// 消息体（标准输出）——m
class MessageFormatItem:public LogFomatter::FormatItem{
public:
    MessageFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getContent();
    }

};
//优先级——p
class LevelFormatItem:public LogFomatter::FormatItem{
public:
    LevelFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<LogLevel::ToString(level);
    }

};
//累积毫秒数——r
class ElapseFormatItem:public LogFomatter::FormatItem{
public:
    ElapseFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getElapse();
    }

};
//日志名称——c
class NameFormatItem:public LogFomatter::FormatItem{
public:
    NameFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getLogger()->getName();
    }

};
//产生该日志的线程号——t
class ThreadFormatItem:public LogFomatter::FormatItem{
public:
    ThreadFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getThreadID();
    }

};
//协程ID——F
class FiberIDFormatItem:public LogFomatter::FormatItem{
public:
    FiberIDFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getFiberID();
    }

};
//时间——d
class DateTimeFormatItem:public LogFomatter::FormatItem{
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S"):m_format(format){
        if(m_format.empty()){
            m_format="%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        struct tm tm;
        time_t time=event->getTime();
        localtime_r(&time,&tm);
        char buf[64];
        strftime(buf,sizeof(buf),m_format.c_str(),&tm);
        std::string str=buf;

        os<<str;
    }
private:
    std::string m_format;       // 表示时间的固定格式
};
//文件名——f
class FilenameFormatItem:public LogFomatter::FormatItem{
public:
    FilenameFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getFile();
    }

};
//行号——l
class LineFormatItem:public LogFomatter::FormatItem{
public:
    LineFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getLine();
    }

};
//换行符——n
class NewLineFormatItem:public LogFomatter::FormatItem{
public:
    NewLineFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<std::endl;
    }

};
//Tab 输出
class TabFormatItem:public LogFomatter::FormatItem{
public:
    TabFormatItem(const std::string& str=""){};
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<"\t";
    }

};
// init中三元组type=0时，直接字符串输出;type=1时用于向终端返回错误
class StringFormatItem:public LogFomatter::FormatItem{
public:
    StringFormatItem(const std::string& str="")
        :m_string(str){}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<m_string;
    }
private:
    std::string m_string;
};

void LogFomatter::init(){
    // str:格式内容项标识符, format:模板内容(无实际作用，仅为初始化类), type:O——非格式内容(非格式内容，"",0)，1——格式内容
    std::vector<std::tuple<std::string,std::string,int> > vec;      //依次存储内容项的三元组
    std::string nstr;        // 非格式内容

    // 字符串处理
    for(size_t i=0;i<m_patten.size();++i){
        if(m_patten[i] != '%'){
            nstr.append(1,m_patten[i]);
            continue;
        }

        if((i+1)<m_patten.size() && m_patten[i+1]=='%'){
            nstr.append(1,'%');
            continue;
        }

        // 以下 m_patten[i]='%'
        size_t n=i+1;
        int fmt_status=0;       // 状态量
        size_t fmt_begin=0;
        std::string fmt;        // 内容模板
        std::string str;        // 格式标识符

        while(n<m_patten.size()){
            // if(isspace(m_patten[i])){
            //     break;
            // }
            if(!fmt_status && !isalpha(m_patten[n]) && m_patten[n]!='{' && m_patten[n]!='}'){
                str=m_patten.substr(i+1,n-i-1);     //TODO 这里怎么是n-i-1呢
                break;
            }
            if(fmt_status==0){
                if(m_patten[n]=='{'){
                    str=m_patten.substr(i+1,n-i-1);
                    fmt_status=1;   // 解析格式 
                    fmt_begin=n;
                    ++n;
                    continue;
                }
            }else if(fmt_status==1){
                if(m_patten[n]=='}'){
                    fmt=m_patten.substr(fmt_begin+1,n-fmt_begin-1);
                    fmt_status=0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n==m_patten.size()){
                if(str.empty()){
                    str=m_patten.substr(i+1);
                }                
            }
        }

        if(fmt_status==0){
            if(!nstr.empty()){
                vec.push_back(std::make_tuple(nstr,std::string(),0));       // 改动：将 "" 改为string的临时变量
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str,fmt,1));
            i=n-1;
        }else if(fmt_status==1){
            std::cout<<"patten parse error: "<<m_patten<<"-"<<m_patten.substr(i)<<std::endl;
            vec.push_back(std::make_tuple("<<patten_error>>",fmt,0));
            m_error=true;
        }
    }
    if(!nstr.empty()){
        vec.push_back(std::make_tuple(nstr,"",0));
    }


    // 日志格式解析到 m_items

    // 格式标识符和格式内容项映射表
    std::unordered_map<std::string,std::function<FormatItem::ptr(const std::string& str)> > s_format_items={
        #define XX(str,C) {#str,[](const std::string& fmt){return FormatItem::ptr(new C(fmt));}}
            XX(m,MessageFormatItem),    // %m————消息体
            XX(p,LevelFormatItem),      // %p————优先级（level）
            XX(r,ElapseFormatItem),     // %r————累积毫秒数
            XX(t,ThreadFormatItem) ,    // %t————产生该日志的线程号
            XX(c,NameFormatItem),       // %c————日志名称
            XX(n,NewLineFormatItem),    // %n————换行符
            XX(d,DateTimeFormatItem),   // %d————时间
            XX(l,LineFormatItem),       // %l————行号
            XX(f,FilenameFormatItem),   // %f————文件名
            XX(F,FiberIDFormatItem),    // %F————协程ID     
            XX(T,TabFormatItem)         // %T————Tab
        
        #undef XX
    };

    for(auto i:vec){
        if(std::get<2>(i)==0){
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }else{
            auto iter=s_format_items.find(std::get<0>(i));
            if(iter==s_format_items.end()){
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format: "+std::get<0>(i)+" >>")));
            }else{
                m_items.push_back(iter->second(std::get<1>(i)));        // 这里传参为什么不对???
            }
        }
        // std::cout<<'{'<<std::get<0>(i)<<"}--{"<<std::get<1>(i)<<"}--{"<<std::get<2>(i)<<'}'<<std::endl;
    }
}

std::string LogFomatter::format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    std::stringstream ss;
    for(auto i: m_items){
        i->format(ss,logger,level,event);       // 这里ss参数是从基类引用到派生类引用的转化
    }
    return ss.str();
}

LogManager::LogManager(){
    m_root.reset(new Logger());
    m_root->addappender(LogAppender::ptr(new StdoutLogAppender));
    m_logMap[m_root->getName()]=m_root;
}

Logger::ptr LogManager::getLogger(const std::string& name){
    auto iter=m_logMap.find(name);
    if(iter==m_logMap.end()){
        // 不在就新建一个
        Logger::ptr logger=std::make_shared<Logger>(Logger(name));
        logger->addappender(LogAppender::ptr(new StdoutLogAppender));
        m_logMap[name]=logger;
    }
    return m_logMap[name];
}


}
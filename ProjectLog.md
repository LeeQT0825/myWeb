
## log
- 输出流式风格日志，支持日志格式、日志级别自定义，支持多日志输出路径。
- 支持时间,线程id,线程名称,日志级别,日志名称,文件名,行号等内容的自由配置。
#### 类图
#### 实现逻辑
1. Logger是最终的日志生成接口，LogEvent为日志事件，包含所有信息。但是LogEvent中包含了Logger的智能指针，为了防止循环引用就不能再在Logger中添加LogEvent智能指针。但是为满足：
- `logger->log(myWeb::LogLevel::WARN,event);`中需要event指针作为参数。
- 使用方便，最好是一个接口。
- 可以调用 *LogEvent::getSS()* 返回日志内容的输出流。
所以使用“宏（便于缩减代码）+LogEventWrap类”
2. 辅助函数库
   - 文件读写类
   - 获取线程号的系统调用：gettid()系统调用；pthread_self() 获取的是用户线程；std::thread::get_id()获取的是用户线程。
3. 时间的输出：重点在一个结构体：struct tm；两个函数：localtime_r(),strftime()。
#### 出现的问题
1. 类的值成员都应该是 'private' 的，所以都应该通过函数接口来访问。
2. 定义宏避免重复 include ：
   ```cpp
   #ifndef __MYWEB_LOG_H__
   #define __MYWEB_LOG_H__
      ...
   #endif
   ``` 
#### 接口
见 test.cc：
```cpp
   SYLAR_LOG_INFO(g_logger) << “this is a log”;
```


## 配置模块
- 定义的地方实现解析
- 配置文件用 *yaml* 格式（其他的格式有 *xml*，*json* 等），从 *yaml* 文件中读取配置数据，数据类型应支持与 *string* 的互相转化。
- 配置模块主要有三个类：*ConfigBase* 基类（提供 *FromString()*,*ToString()* 两个模板函数），*ConfigVar* 类，*Config* 类。

### yaml-cpp


## 线程模块


## 协程
协程是在两个执行栈上切换的，表现为从一个函数的某处**切换到**（并非调用）另一个函数的**某处**（而不是入口）。
### 协程模块
一个原子的执行单元。 
### 协程调度模块
负责协程的生命周期、创建、销毁、调度。


## IO协程调度模块
基于 schedual 和 epoll。 


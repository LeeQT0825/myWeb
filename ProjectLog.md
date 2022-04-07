
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
### boost/lexical_cast.hpp
- Boost 库是一个开源的，可移植的“准”标准库，是 STL库 的补充
- Boost 库中的 lexical_cast 为数值之间的转换（conversion）提供了一个更好的方案，建议忘掉std诸多的函数，直接使用lexical_cast，如果转换发生了意外，lexical_cast 会抛出一个 bad_lexical_cast 异常，因此程序中需要对其进行 try-catch 。
### RTTI（Run-Time Type Identification）——运行时类型识别
在C++中，为了支持RTTI提供了两个操作符：dynamic_cast 和 typeid：
   - dynamic_cast允许运行时刻进行类型转换，从而使程序能够在一个类层次结构中安全地转化类型，与之相对应的还有一个非安全的转换操作符static_cast。
   - typeid是C++的关键字之一，等同于sizeof这类的操作符。typeid操作符的返回结果是名为type_info的标准库类型的对象的引用（在头文件typeinfo中定义，稍后我们看一下vs和gcc库里面的源码），它的表达式有下图两种形式。
### yaml-cpp
### 出现的问题
- 在编译过程中出现 “undefined reference to `vtable for...' ”的问题，可能的原因如下：
  - 子类没有实现父类的纯虚函数
  - 父类的析构函数应为虚函数，以满足子类释放内存。
      1）注意父类的析构函数、子类的构造函数不应只声明，必须要实现，哪怕仅仅是 {} 。
      2）子类数据成员初始化时应先将父类的构造函数初始化。


## 线程模块


## 协程
协程是在两个执行栈上切换的，表现为从一个函数的某处**切换到**（并非调用）另一个函数的**某处**（而不是入口）。
### 协程模块
一个原子的执行单元。 
### 协程调度模块
负责协程的生命周期、创建、销毁、调度。


## IO协程调度模块
基于 schedual 和 epoll。 



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
- 约定优于配置：
&emsp;系统有默认值，非必要可以不做配置。
- 配置文件用 *yaml* 格式（其他的格式有 *xml*，*json* 等），从 *yaml* 文件中读取配置数据，数据类型应支持与 *string* 的互相转化。
- 配置模块主要有三个类：*ConfigBase* 基类（提供 *FromString()*,*ToString()* 两个模板函数），*ConfigVar* 类，*Config* 类。
### 相关技术要点
#### yaml-cpp
- .yml 文件是层级结构，由 Map、Sequence、Schalar 组成。因为是层级的数据结构，所以可以通过DFS遍历Node方式读取全部数据
   ```cpp
   if(node.isMap()){
      for(auto iter=node.begin();iter!=node.end();++iter){
         iter->first    // string 类型
         iter->second   // Node 类型
      }
   }
   
   if(node.isSequence()){
      for(int i=0;i<node.size();++i){
      }
   }
   ```
- 加载.yml文件：YAML::NODE LoadFile(filename);    
- 转换.yml文件：要根据层级结构转换成对应的数据结构关系。
#### 类型转换
##### boost/lexical_cast.hpp
- Boost 库是一个开源的，可移植的“准”标准库，是 STL库 的补充
- Boost 库中的 lexical_cast 为数值之间的转换（conversion）提供了一个更好的方案，建议忘掉std诸多的函数，直接使用lexical_cast，如果转换发生了意外，lexical_cast 会抛出一个 bad_lexical_cast 异常，因此程序中需要对其进行 try-catch 。
##### RTTI（Run-Time Type Identification）——运行时类型识别
&emsp;在C++中，为了支持RTTI提供了两个操作符：dynamic_cast 和 typeid：
   - dynamic_cast允许运行时刻进行类型转换，从而使程序能够在一个类层次结构中安全地转化类型，与之相对应的还有一个非安全的转换操作符static_cast。
   - typeid是C++的关键字之一，等同于sizeof这类的操作符。typeid操作符的返回结果是名为type_info的标准库类型的对象的引用（在头文件typeinfo中定义，稍后我们看一下vs和gcc库里面的源码），它的表达式有下图两种形式。
#### 回调
&emsp;当一个配置发生修改的时候，可以反向通知调用它的代码。实现方法：
   - 配置的事件机制：
      使用配置变量的函数向 cbMap 中注册函数指针，当配置变量值发生变化的时候，通过函数指针通知回调函数。
   - **变更回调函数组** 用 *map* 而不用 *vector* 的原因：
      因为 std::function<> 类没有比较函数（或比较操作符的重载），所以用 vector 无法判断两个 function 是否是一样的。
      想要删除一个 function 的时候，map 可以通过删除 key 来删除。
   - unit64_t 类型的 key 要求唯一，一般可以用hash值
  
&emsp;每个模块实现从配置变量回调：
   - 确定每个模块需要配置的变量。如log模块：LogAppenderDefine类、LogDefine类
   - 上述类均要提供 "= ="、"<" 等符号的重载。
      "= =": 提供新旧配置变量值的对比
      "<": 提供在set、map等容器内的 Find() 的地层实现。
   - 要提供上述类和 YAML::string 的类型转换
   - 每个模块要定义自己的 ConfigVar 。
   - 每个模块初始化配置变量（这里用全局变量，因为全局变量在 main() 前初始化，一定会触发），初始化的内容是向 Config 类中 addListener(std::function<>()) 。
### 出现的问题
- 在编译过程中出现 “undefined reference to `vtable for...' ”的问题，可能的原因如下：
  - 子类没有实现父类的纯虚函数
  - 父类的析构函数应为虚函数，以满足子类释放内存。
      1.  注意父类的析构函数、子类的构造函数不应只声明，必须要实现，哪怕仅仅是 {} 。
      2.  子类数据成员初始化时应先将父类的构造函数初始化。


## 线程模块
&emsp;选择自己封装一个线程类的原因：
* C++11中提供了std::thread, std::mutex, std::condition_variable等线程相关的类。但是还是不够完善，比如没有提供读写锁。没有提供spinlock，虽然c++11里面的atomic可以用来实现CAS锁。
* 对于高并发服务器来说，很多数据，大多数时候，都是写少读多的场景。如果没有读写锁的话，那么性能上的损失会非常大。
* thread库也是基于pthread实现的，所以不如自己封装一个适合本项目的线程类。

### 相关技术要点
#### pthread 和 thread
- pthread 早于 thread。
- thread是C++的API, 不可以在C++中调用，换句话说，它更加简单和安全。 它大量使用RAII来确保资源在超出范围时得到回收，并允许您从函数对象创建一个线程，具有所有的灵活性，而不是被限制为C风格的自由函数。
&emsp;但是thread也有很多被人吐槽的地方：
   - C++11 thread库里居然没有shared_mutex。
   - C++11 thread库居然没有executor之类的thread pool
   - 在Linux下，C++11 thread库居然强制动态连接pthread，如果你编译连接的时候忘了-pthread参数，一直要到运行的时候才会报错
- pthread是一个C的API，因此它不提供任何RAII，这使得它更难使用，更容易出错，特别是就异常安全性而言（除非你自己编写了包装，你现在必须调试 并保持）。
#### 自定义的线程类
&emsp;自定义的线程类应具备一下特点：
1. 不可被拷贝（拷贝构造函数设置为 private ，且函数声明后加“ ***=delete*** ”---不可被调用）
2. 

## 协程
&emsp;协程是在两个执行栈上切换的，表现为从一个函数的某处**切换到**（并非调用）另一个函数的**某处**（而不是入口）。
### 协程模块
&emsp;一个原子的执行单元。 
### 协程调度模块
&emsp;负责协程的生命周期、创建、销毁、调度。


## IO协程调度模块
&emsp;基于 schedual 和 epoll。 


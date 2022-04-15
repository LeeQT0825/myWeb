
## log
- 输出流式风格日志，支持日志格式、日志级别自定义，支持多日志输出路径。
- 支持时间,线程id,线程名称,日志级别,日志名称,文件名,行号等内容的自由配置。
### 类图
### 实现逻辑
1. Logger是最终的日志生成接口，LogEvent为日志事件，包含所有信息。但是LogEvent中包含了Logger的智能指针，为了防止循环引用就不能再在Logger中添加LogEvent智能指针。但是为满足：
- `logger->log(myWeb::LogLevel::WARN,event);`中需要event指针作为参数。
- 使用方便，最好是一个接口。
- 可以调用 *LogEvent::getSS()* 返回日志内容的输出流。
所以使用“宏（便于缩减代码）+LogEventWrap类”
2. 辅助函数库
   - 文件读写类
   - 获取线程号的系统调用：gettid()系统调用；pthread_self() 获取的是用户线程；std::thread::get_id()获取的是用户线程。
3. 时间的输出：重点在一个结构体：struct tm；两个函数：localtime_r(),strftime()。
4. 加锁：（写多读少）
   - LogAppender 输出时需要加锁。 
### 接口
见 test.cc：
```cpp
   SYLAR_LOG_INFO(g_logger) << “this is a log”;
```
### 出现的问题
1. 类的值成员都应该是 'private' 的，所以都应该通过函数接口来访问。
2. 定义宏避免重复 include ：
   ```cpp
   #ifndef __MYWEB_LOG_H__
   #define __MYWEB_LOG_H__
      ...
   #endif
   ``` 

## 配置模块 
- 约定优于配置：
&emsp;系统有默认值，非必要可以不做配置。
- 配置文件用 *yaml* 格式（其他的格式有 *xml*，*json* 等），从 *yaml* 文件中读取配置数据，数据类型应支持与 *string* 的互相转化。
### 实现逻辑
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

## 信号量、互斥量模块
### 实现逻辑
#### 自定义类
- 自定义的信号类应具备以下特点：
  1. 不可被拷贝构造，赋值构造等。
  2. 封装信号量的基本功能。
- 自定义的锁类应具备以下特点：
  1. 局部定义锁应满足 RAII 规范。
  2. 不可被拷贝构造，赋值构造等。
  3. 各种锁设定统一的接口。
#### 需要加锁的地方
- 线程不安全的地方，如类的成员变量会引起成员函数的竞态时：如果可能会有多个线程同时想修改同一个类的成员变量的时候。
- 
### 相关技术要点
#### 互斥量的相关细节
#### 锁的相关细节
- 互斥锁保证了线程间的同步，但是却将并行操作变成了串行操作，这对性能有很大的影响，所以我们要尽可能的减小锁定的区域（临界区），也就是使用细粒度锁。
- 临界区管理加锁或解锁的有两个类（lock_guard<'mutex type'> , unique_lock<'mutex type'>）：
  - **lock_guard<'mutex type'>**:
    1. lock_guard对象只能保证在析构的时候执行解锁操作，且本身并没有提供加锁和解锁的接口，不够灵活。
    2. lock_guard对象在析构的时候就一定会解锁，锁的力度过大。
  - **unique_lock<'mutex type'>**:
    1. unique_lock对象提供了lock()和unlock()接口，能记录现在处于上锁还是没上锁状态，在析构的时候，会根据当前状态来决定是否要进行解锁。
    2. 在无需加锁的操作时，可以先临时释放锁，然后需要继续保护的时候，可以继续上锁，这样就无需重复的实例化lock_guard对象，还能减少锁的区域。
    3. 它内部需要维护锁的状态，所以效率要比lock_guard低一点。
  - unique_lock和lock_guard都不能复制，lock_guard不能移动，但是unique_lock可以。
- 原子锁：
  - 当我们修改某一个变量的时候，在汇编层面看来，至少需要细分为“读->改->写”三个过程，也就是说，他们访问存储单元两次，第一次读原值，第二次写新值。两次访问存储单元的操作可能会被中断，所以我们希望将两个操作合为原子操作。
  - 锁实际上挂起了线程执行，释放了CPU资源来执行其他任务，但是在停止/重新启动线程时会产生明显的上下文切换开销。
   相反，尝试原子操作的线程不会等待并一直尝试直到成功（所谓的繁忙等待），因此它们不会招致上下文切换开销，但不会释放CPU资源。
  - https://my.oschina.net/u/3707404/blog/3211668 
   
## 线程模块
&emsp;选择自己封装一个线程类的原因：
* C++11中提供了std::thread, std::mutex, std::condition_variable等线程相关的类。但是还是不够完善，比如没有提供读写锁。没有提供spinlock，虽然c++11里面的atomic可以用来实现CAS锁。
* 对于高并发服务器来说，很多数据，大多数时候，都是写少读多的场景。如果没有读写锁的话，那么性能上的损失会非常大（std::thread 没有读写锁）。
* thread库也是基于pthread实现的，所以不如自己封装一个适合本项目的线程类。
### 实现逻辑
- 线程由 pthread_create() 创建。
- Thread 类的静态成员函数 static void Th_Create(void*) 用来满足 pthread_create() 的第三个参数：void* (*start_routine)(void*) ，第四个参数是第三个参数的参数值，就是 Thread 类的 this 。
- this 的类成员变量初始化在构造函数阶段完成。
- 加信号量：构造函数那里要保证同步性，即类的构造函数应该在线程的执行函数之前完成构造。
#### 自定义的线程类
&emsp;自定义的线程类应具备一下特点：
1. 不可被拷贝（拷贝构造函数设置为 private ，且函数声明后加“ ***=delete*** ”---不可被调用）
2. 该类在构造函数中就执行新的线程，从类的静态函数进入，所以类对象的作用域依然是父线程。
3. 类对象的创建（所有成员对象的初始化，和构造函数完成）要在新线程陷入执行函数之前完成。
4. 类的析构函数需要在子线程执行结束前决定到底是 join 还是 detach 。
### 相关技术要点
#### pthread 和 thread
- pthread 早于 thread。
- thread是C++的API, 不可以在C++中调用，换句话说，它更加简单和安全。 它大量使用RAII来确保资源在超出范围时得到回收，并允许您从函数对象创建一个线程，具有所有的灵活性，而不是被限制为C风格的自由函数。
&emsp;但是thread也有很多被人吐槽的地方：
   - C++11 thread库里居然没有shared_mutex。
   - C++11 thread库居然没有executor之类的thread pool
   - 在Linux下，C++11 thread库居然强制动态连接pthread，如果你编译连接的时候忘了-pthread参数，一直要到运行的时候才会报错
   - thread 中没有读写分离的互斥量（读写锁）
- pthread是一个C的API，因此它不提供任何RAII，这使得它更难使用，更容易出错，特别是就异常安全性而言（除非你自己编写了包装，你现在必须调试 并保持）。
#### 线程其他
1. **线程安全** 一个线程安全的类(class)应当满足三个条件: 
   1. 多个线程同时访问资源时, 其表现出正确的行为。
   2. 无论操作系统如何调度这些线程, 无论这些线程的执行顺序如何交织。
   3. 调用端的代码无需额外的同步或其他协调动作。
2. 线程从它被创建的时候就开始执行它的新线程了。
3. pthread_setname_np() 函数可用于为线程（或其他线程）设置唯一名称，这对于调试多线程应用程序非常有用。 线程名称是一个有意义的 C 语言字符串，包括终止空字节 ('\0')在内，其长度限制为 16 个字符。成功时，这些函数返回 0； 出错时，它们返回一个非零错误号。



## 协程
&emsp;协程是在两个执行栈上切换的，表现为从一个函数的某处**切换到**（并非调用）另一个函数的**某处**（而不是入口）。
### 协程模块
&emsp;一个原子的执行单元。 
### 协程调度模块
&emsp;负责协程的生命周期、创建、销毁、调度。


## IO协程调度模块
&emsp;基于 schedual 和 epoll。 


## Question
1. 如何保证STL容器的迭代器在多线程中不会失效？如 Logger 中的 appenders 成员变量，在 log() 和 addAppender() 都会对appenders进行修改，这时 log() 中的迭代器是否会失效？
2. 在 Logger 类中加入互斥锁后，在 LogManager::getLogger() 中使用 make_shared 创建 Logger ，显示 “error: use of deleted function ‘myWeb::Logger::Logger(myWeb::Logger&&)’” ？
   - A: 互斥锁是不可赋值、不可移动构造、不可赋值构造的，作为 Logger 的成员变量，向上影响到 Logger 也变为不可赋值、不可移动构造、不可赋值构造的。而 make_shared 的可变参数传入的是移动构造的对象，所以发生错误。
   - S: 将 make_shared 改为 shared_ptr<> obj() 直接构造智能指针。

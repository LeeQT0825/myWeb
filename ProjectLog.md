## 高性能服务器
### 要解决的问题
- 减少线程的重复高频创建————线程复用 
   常规解决办法：线程池
- 尽量避免线程的阻塞————IO多路复用、引入线程池
  - Reactor && Proactor（非阻塞回调），解决问题的能力有限
  - 响应式编程，容易陷入回调地狱，割裂业务逻辑
  - 其他方法，例如协程
- 提升代码的可维护与可理解性，尽量避免回调地狱 
-  ```
   IO模型(5种)————>事件处理模型（基于同步IO、异步IO）————>并发模型
                      |                                |
               可读、可写等IO事件                      编程模式{半同步／半异步(包括其演变模式)、Follower／Leader模式}
                      |
            Reactor模式、Proactor模式
   ```
- Reactor的核心思想：将关注的I/O事件注册到多路复用器上，一旦有I/O事件触发，将事件分发到事件处理器中，执行就绪I/O事件对应的处理函数中。模型中有三个重要的组件：
   - **多路复用器**：由操作系统提供接口，Linux提供的I/O复用接口有select、poll、epoll；
   - **事件分发器**：将多路复用器返回的就绪事件分发到事件处理器中；
   - **事件处理器**：处理就绪事件处理函数

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
  - 系统有默认值，非必要可以不做配置。
  - 系统原本没有的配置变量，及时 .yml 文件中有，也不会配置（即只配置已存在的配置变量）。
  - 为实现第二点，在加入配置模块的时候应该初始化配置：
    - 即定义一个配置变量对象，考虑配置值的类型：配置值本身做配置系统一个map元素键值对中的值，但配置值分支是 map 还是 sequence 。
- 配置文件用 *yaml* 格式（其他的格式有 *xml*，*json* 等），从 *yaml* 文件中读取配置数据，数据类型应支持与 *string* 的互相转化。
### 实现逻辑
- 配置模块主要有三个类：*ConfigBase* 基类（提供 *FromString()*,*ToString()* 两个模板函数），*ConfigVar* 类，*Config* 类。
- 如果配置变量是 Sequence ，则定义配置变量的时候应该定义成 set<T> （如 set<LogDefine>）。
- 初始化配置变量：
  - 通过 Config::Lookup() 的重载定义一个配置变量 ConfigVar<T> ；
  - 定义一个结构体，来初始化全局变量并添加 addListener() ，定义好后，用这个结构体定义一个静态变量（实现在main之前运行）
- Config 类基本都是静态成员，静态成员的初始化顺序是随机的，所以如果 m_lock 锁不是静态变量的话，会导致其他函数进行的时候他还没初始化好的情况。
### 相关技术要点
#### yaml-cpp
- .yml 文件是层级结构，由 Map(使用‘:’表示键值对(注意：一个键值对是一个map元素))、Sequence(使用‘-’)、Schalar 组成。因为是层级的数据结构，所以可以通过DFS遍历Node方式读取全部数据
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
     - static_cast 进行上行转换（派生类->基类）时是安全的，而下行转换时是不安全的。
     - dynamic_cast 进行上行转换时和 static_cast 效果一样，在进行下行转换时会先进行类型检查，所以比 static_cast 更安全。
     - dynamic_pointer_cast 与 dynamic_cast 用法类似。当指针是智能指针时候，向下转换，用 dynamic_cast 则编译不能通过，此时需要使用 dynamic_pointer_cast。
   - typeid是C++的关键字之一，等同于sizeof这类的操作符。typeid操作符的返回结果是名为type_info的标准库类型的对象的引用（在头文件 *\<typeinfo\>* 中定义）
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
- 32位系统，每创建一个进程，会分配 4G 大小的内存空间，其中 1G 为内核空间，3G 为用户空间。
- 进程每创建一个线程，就会为线程分配一个**栈大小**的内存，每个操作系统栈大小上限不一样，通常 8M 或 10M （可用用“*ulimit -s*”指令查看，也可通过该指令进行设置）。
- 线程太多也会有缺页中断的风险。综上2-3点，进程不可以分配太多的线程。
&emsp;选择自己封装一个线程类的原因：
* C++11中提供了std::thread, std::mutex, std::condition_variable等线程相关的类。但是还是不够完善，比如没有提供读写锁。没有提供spinlock，虽然c++11里面的atomic可以用来实现CAS锁。
* 对于高并发服务器来说，很多数据，大多数时候，都是写少读多的场景。如果没有读写锁的话，那么性能上的损失会非常大（std::thread 没有读写锁）。
* thread库也是基于pthread实现的，所以不如自己封装一个适合本项目的线程类。
### 实现逻辑
- 线程由 pthread_create() 创建。(创建的是TWP)
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
   2. 无论操作系统如何调度这些线程, 无论这些线程的执行顺序如何交织，都能正常运行。
   3. 调用端的代码无需额外的同步或其他协调动作。
2. 线程从它被创建的时候就开始执行它的新线程了。
3. pthread_setname_np() 函数可用于为线程（或其他线程）设置唯一名称，这对于调试多线程应用程序非常有用。 线程名称是一个有意义的 C 语言字符串，包括终止空字节 ('\0')在内，其长度限制为 16 个字符。成功时，这些函数返回 0； 出错时，它们返回一个非零错误号。



## 协程
### 什么是协程
#### 概念理解
- 协程可以用同步编程的方式实现异步编程(很大的作用是剔除线程的阻塞)才能实现的功能。因为实现异步编程的效果，所以协程也可以剔除IO阻塞。虽然不能减少一次业务请求的耗时，但可以提升系统的吞吐量。
- 协程是在两个执行栈上切换的，表现为从一个函数的某处**切换到**（并非调用）另一个函数的**某处**（而不是入口），简单来说可以认为协程是线程里不同的函数，这些函数之间可以相互快速切换。所以协程和用户态的线程非常接近。
- 协程运行在线程之上，是线程的线程，比线程更轻量级。
- 协程适用于IO密集型的任务，对计算密集型的任务反而增加了协程切换的开销。如果业务处理时间远小于 IO 耗时，线程切换非常频繁，那么使用协程是不错的选择。
- 常见提供原生协程支持的语言有：c++20、golang、python 等，其他语言以库的形式提供协程功能，比如 C++20 之前腾讯的 fiber 和 libco 等等。
- 协程比线程的优点：
  1. 系统线程会占用大量的内存空间，假设一个线程至少占用 4M 内存，10000个线程就会消耗掉 39G 内存。而协程并没有增加线程数量，只是在线程的基础之上通过分时复用的方式运行多个协程。
  2. 线程调度会占用大量的系统时间 。而**协程的切换在用户态完成**，切换的代价比线程从用户态到内核态的代价小很多。
   - 综上，协程不仅减少了线程的切换，从编程角度讲，协程的引入简化了异步编程。
- 假设协程运行在线程之上，并且协程调用了一个阻塞IO操作，由于操作系统并不知道协程的存在，它只知道线程，因此在协程调用阻塞IO操作的时候，操作系统会让线程进入阻塞状态，当前的协程和其它绑定在该线程之上的协程都会陷入阻塞而得不到调度，这是不可接受的。因此在协程中不能调用导致线程阻塞的操作。也就是说，**协程只有和异步IO结合起来，才能发挥最大的威力**。
#### 有栈协程和无栈协程
这里的栈是逻辑栈
- 有栈协程
   协程 A 调用了协程 B，如果只有 B 完成之后才能调用 A 。此时 A 和 B 是非对称协程。在非对称协程中，可以借助专门的调度器来负责调度协程，每个协程只需要运行自己的入口函数，然后结束时将运行权交回给调度器，由调度器来选出下一个要执行的协程即可。
- 无栈协程
   A/B 被调用的概率相同。此时 A 和 B 是对称协程。子协程可以直接和子协程切换，也就是说每个协程不仅要运行自己的入口函数代码，还要负责选出下一个合适的协程进行切换，相当于每个协程都要充当调度器的角色，这样程序设计起来会比较麻烦，并且程序的控制流也会变得复杂和难以管理。
### 协程模块
- 一个线程的栈大小比较大，但是一个协程的栈大小通常比较小（因为需要快速切换协程，且通常具有好几个协程）且可以设置。因此**通常不会在协程中创建很大的对象**，因为自动对象都是创建在栈上，对象过大会产生栈溢出的错误。**能用指针的尽量用指针**。
- ucontext 机制是 GNU C 库提供的一组用于创建、保存、切换用户态执行“上下文”（context）的API。
- 基于 ucontext_t 实现：
  - ucontext_t 结构体包的内容：
     ```cpp
     - ucontext_t *uc_link        //next context，指向当前 context 结束时待恢复的上下文
     - sigset_t uc_sigmask        //信号掩码
     - stack_t uc_stack           //当前上下文所使用的栈
     - mcontext_t uc_mcontext     //实际保存 CPU 上下文的变量，这个变量与平台&机器相关，最好不要访问这个变量
     ```
  - 定义了四个函数：
     ```cpp
     - int  getcontext(ucontext_t * ucp);        // 使用当前 CPU 上下文初始化 ucp 所指向的结构体，成功返回0，失败返回-1
     - int  setcontext(const ucontext_t *);      // 重置当前 CPU 上下文
     - void makecontext(ucontext_t *, (void *)(), int, ...);   // 修改由 getcontext 创建的上下文信息，比如设置栈指针
     - int  swapcontext(ucontext_t *, const ucontext_t *);     // 切换两个上下文
     ```
   https://www.jianshu.com/p/a96b31da3ab0
- 对 ucontext_t 类型的理解：
  1. 这个对象就相当于"传送门"：从一个运行状态传送到另一个状态，且两边的运行状态发现不了执行权已经易主————所以我们需要保存当前状态的栈空间 uc_stack（栈指针和栈大小）。
  2. 由于这是在同一个线程中来回切换，而"传送门"一定要和某个可运行的程序绑定，所以"传送门"一定要和某个function绑定。（这就是 makecontext() 的作用）
  3. 线程执行完有 join() 或 detach() 来辅助线程的关闭。而"传送门"所绑定的function总有执行完的时候，function执行完，"传送门"的另一端就是一片虚无，所以要手动设置function执行完后"传送门"的归宿（设置 uc_link 的作用：设置为 nullptr 则整个线程直接关闭，设置为其他 ucontext_t 对象则传送到其他"传送门"）。
#### 实现逻辑
- 需要实现：
  - 本协程是非对称协程，必须要实现两个原语：
    - resume语义（唤醒协程），就是从主协程切换到目标协程，此时主协程让出CPU，目标协程抢占CPU，然后在目标协程的空间上执行目标协程的代码。
    - yield语义（挂起协程），就是从当前协程切换回到主协程。
  - 每个线程都要先有一个 t_mainFiber 来创建管理调度其他子协程。（可以改进的点：直接由本协程调度）
    - 主协程：负责创建、调度
      - 是线程中第一个协程，由 getThis() 创建并返回主协程智能指针。
      - 不分配栈空间
      - 没有回调
      - 状态保持在EXEC。
    - 子协程：执行完或者让出时间片只能回到主协程，再由主协程调度。
      - 有回调函数，需要分配栈空间。
  - 调度：因为实际应用时，都是 Scheduler 来统一调配所有线程的协程。根据 Scheduler 的两种模式：
    - use_caller（默认true）：除 Scheduler 的主线程，线程池中的线程下所有协程的转换都是**当前协程**和该线程的**调度协程**之间的转换，接口是 swapIn(), swapOut()。（因为线程池中线程的主协程都是创建在 run() 中，它们的主协程充当了 t_master_Fiber 的角色）
    - 非use_caller（false）：Scheduler 主线程中的**主调度协程**应是该模式下创建，是**当前协程**和**主协程**之间的转换，接口是 call(), back()。（因为 Scheduler 的主协程创建在 main 函数中，执行 run() 的协程是 ID=1 的子协程）
### 协程调度模块
#### N:1 && N:M 协程
- N:1 模式：我们常说的协程特指 N:1 线程库，即所有的协程运行于一个系统线程中。由于不跨线程，协程之间的切换不需要系统调用，可以非常快(100ns-200ns)，受cache一致性的影响也小。但代价是协程无法高效地利用多核，**代码必须非阻塞**，否则线程中所有的协程都被卡住。缺点是诸多线程无法负载均衡（导致一个线程中的某个协程运行过久，会影响到本线程的其他协程）。
- N:M 模式：协程可以跨线程执行，该模式下的协程其实就是可用户确定调度顺序的用户态线程。
#### 实现逻辑
- 成员变量：
  - 主线程（use_caller）————是调度器的调用线程
  - 主调度协程————主线程的调度协程（m_rootFiber），执行 run()
  - 线程池————可供调度器调度的目标线程
  - 执行队列
- 两种模式：
  - use_caller=true：调用线程不加入线程池中
  - use_caller=false：
```
   1      :    M  , 1    :     N
schedule ————> thread ————> fiber
```
- schedule 功能：
  - 线程池，分配一组线程
  - 执行队列 list<FiberandThread>————线程池中的线程在执行队列上循环
    - 如果是fiber，则直接调度。
    - 如果是func，则通过 cb_fiber 调度。
    - 如果没有执行任务
      - 如果执行队列未空，则是没有匹配到任务————继续循环
      - 如果执行队列为空，则通过 idle() 检查 Scheduler 是否已经停止————等到最后一个任务执行完一起结束（防止期间又有任务进来却没有线程执行）
    - 同一个线程可以创建多个协程，协程可以被不同线程调用执行（也可以通过 Scheduler::schedule() 指定线程执行）
  - 重点和难点： 
    - 协程调度的转换：call() back() swapIn() swapout() 的调度
    - use_caller 模式下 m_rootFiber 启动时间
        不应该在 start() 阶段启动，否则主线程将全部时间都陷入到 run() ，不会再执行主线程内后续添加的任务。
    - 如何实现某一协程在 HOLD 下保存生命，且可以被不同线程随时调度：
        在 Scheduler::run() 中 cb_fiber 是 Fiber::ptr 类型，就是个指针。如果 cb_fiber 所指的fiber在yield后状态为 HOLD ，则这个fiber不能让它销毁，下一次循环取执行函数时，只需把 cb_fiber.reset() ，让它重新创建一个新的 fiber 来承载执行函数。之前 HOLD 的fiber可以保留指针，用定时器在合适的时间通过指针唤醒（hook的sleep就是这么实现的）。这样就可以
#### 出现的问题
1. scheduler.cc ——— start() 函数中，m_rootFiber->swapIn() 时产生了死锁。产生原因：
2. **stack smashing detected** ：程序访问了非法的栈空间
   - 数组越界
   - 临时变量已经释放了，但是还在访问这块内存。

## 定时器
### 实现逻辑
- 因为 epoll_wait 超时时间是毫秒级，所以定时器的精度也是毫秒级。
- Timer 是定时器类，由 TimerManager 类控制。
- 用 set 实现定时器排序的问题（红黑树），排序规则是 m_NextTime 绝对时间的比较。
- 定时器的意义在于它的回调函数。将定时器取消要做到就是将 timer->m_cb 置为 nullptr 。
- 在 util.cc 中定义了获取当前时间的函数 GetCurrentMS() ，表示时间的数据类型是 timeval (普通精度，微秒+秒)
### 出现的问题
- 时间戳校准：时间戳来源:
  原来的时间源是 <sys/time.h> 的 gettimeofday() 。后发现**需要校对时间**，因为如果用户修改了系统时间，则定时器失灵。

## IO协程调度模块
&emsp;基于 schedual 和 epoll。线程池的线程在没有执行任务的时候，在 idle() 中阻塞在 epoll_wait() 中，有任务需要执行的时候，被 tickle() 从 idle() yield出来。
### 相关知识点
- &emsp;通常线程池协作的实现方法是通过信号量维护一个消息队列（生产者消费者模式）来通知各个线程。但产生的问题是，如果某线程拿不到信号量，就会一直阻塞在等待队列中，性能下降会很大。
&emsp;通过异步IO，当线程阻塞在 epoll_wait() 的时候，如果有需要执行的任务就切换到任务处，这两部分通过 pipe() 实现。
- 异步IO库：可以省略判断 socket fd 上发生的是可写还是可读，通过在可写或可读状态下绑定回调函数实现（FdContext就是这个功能）
  - 包含两个核心：
    - 在子线程/其他进程上执行IO操作
    - 在执行完毕后通知调用者提取相关数据
  - 实现原理：
    - 先将套接字设置成非阻塞状态，然后将套接字与回调函数绑定，接下来进入一个基于IO多路复用的事件循环，等待事件发生，然后调用对应的回调函数。
### 实现逻辑
- 实现的模式：
  - IO模型：基于epoll实现的IO复用
  - 事件处理模式：模拟 Proactor 模式
  - 并发模式：
- 事件分类：
  - 继承自 EPOLL 的事件，将所有事件分类为**可读(0x001)**、**可写(0x004)**，所以事件回调也只分为 Read_cb 和 Write_cb ，如果 epoll_wait 返回错误或中断，则触发该 fd 的全部事件。
- 事件处理：
  - FdContext 类
    - 封装了异步模式下注册事件需要的三元组：fd 、Event(事件：可读、可写) 、EventCallBack(事件回调：用类封装了调度器和回调执行函数) 。
    - 提供对事件的操作：增加事件、删除事件、取消事件、触发事件
    - 当事件触发后，该 fd 的相应事件从 epoll_wait 中删除。注意：listenfd需要特殊处理。
### 出现的问题
  - m_fdcontexts 是将 fd 作下标存储的，通过 epoll 来通知就绪任务。m_ticklefd 也是通过 epoll 来通知的，两个 fd 不会冲突么？m_fdcontexts 会不会爆炸增长？————好像不会，fd都是内核分配的。
  - 如何重复注册listenfd的读事件？应该取出事件马上再重新注册

## HOOK 模块
### HOOK 概述
&emsp;hook实际上就是对系统调用API进行一次封装，将其封装成一个与原始的系统调用API同名的接口，应用在调用这个接口时，会先执行封装中的操作，再执行原始的系统调用API。
&emsp;hook技术可以使应用程序在执行系统调用之前进行一些隐藏的操作，比如可以对系统提供malloc()和free()进行hook，在真正进行内存分配和释放之前，统计内存的引用计数，以排查内存泄露问题。
&emsp;在IO协程调度中对相关的系统调用进行hook，可以让调度线程尽可能得把时间片都花在有意义的操作上，而不是浪费在阻塞等待中。
### 实现原理
&emsp;hook的实现机制非常简单，就是通过动态库的全局符号介入功能，用自定义的接口来替换掉同名的系统调用接口。由于系统调用接口基本上是由C标准函数库libc提供的，所以这里要做的事情就是用自定义的动态库来覆盖掉libc中的同名符号。
- 由于系统调用接口基本是由C标准函数库提供，在编译期间C和C++对函数名称会编译成不同的函数符号（函数签名），所以 hook 的函数都要在 extern "C"{}中定义。extern “C” 的作用是告诉C++编译器用C规则编译指定的代码（除函数重载外，extern “C”不影响C++其他特性）。
### 实现逻辑
- 线程粒度，在 Scheduler::run() 中将hook使能。
- 三类接口需要HOOK：
  1. sleep 延时系列接口，包括 sleep/usleep/nanosleep 。对于这些接口的hook，只需要给IO协程调度器注册一个定时事件，在定时事件触发后再继续执行当前协程即可。当前协程在注册完定时事件后即可 yield 让出执行权。
  2. socket IO 系列接口，包括 read/write/recv/send...等，connect 及 accept 也可以归到这类接口中。这类接口的 hook 首先需要判断操作的fd是否是 socket fd，以及用户是否显式地对该fd设置过非阻塞模式，如果不是 socket fd 或是用户显式设置过非阻塞模式，那么就不需要 hook 了，直接调用操作系统的IO接口即可。如果需要 hook ，那么首先在IO协程调度器上注册对应的读写事件，等事件发生后再继续执行当前协程。当前协程在注册完IO事件即可 yield 让出执行权。
     - 统一的实现接口： 
      ```cpp
      template<typename OriginFunc,typename ... Args>
      static ssize_t do_io(int fd,OriginFunc origin_func,const char* hook_func_name,
                uint32_t event,int timeout_so,Args&& ... args)
      /* 实现逻辑：
        1. 通过 FDctx 类将 fd 设置为非阻塞
        2. 利用原函数获取返回值保存下来。（因为 fd 是非阻塞的，如果未就绪会返回 -1 ，且 errno=EAGAIN，如果被内核中断，则 errno=EINTER）
        3. 如果 errno=EAGAIN ，则先设定条件定时器，回调函数就是将该函数的注册事件（event）取消。（这时还没注册事件，毕竟定时器需要一段时间才能触发，不急）
        4. 注册事件，注册成功后就可以切回调用协程了
        5. 切回当前协程的途径有两种：1.定时器触发，取消事件会强制触发回调。 2.事件就绪，通过 epoll_wait 回调
        6. 切回当前协程后取消定时器，判断有无错误（Timer_Cond::cancelled）
        7. 返回原函数返回值
      ```
  3. socket/fcntl/ioctl/close 等接口，这类接口主要处理的是边缘情况，比如分配fd上下文，处理超时及用户显式设置非阻塞问题。
### 句柄管理器
&emsp;应用hook的场景应该先确定 fd 是否是socketfd、是否阻塞、是否有超时时间等。所以应该有一个句柄管理类来管理 fd 的属性。
- 用 ```int fstat(int fd, struct stat *statbuf)``` 来判断句柄属性。判断 fd 是否为 socketfd 可以通过 S_ISSOCK(fd_stat.st_mode) 来判断。

## Socket API
### 一些 socket API 的底层逻辑
1. ssize_t **send**(int sockfd, const void *buf, size_t len, int flags);
  **同步编程下 send 的执行过程**：
    1. send先比较待发送数据的长度 len 和套接字 sockfd 的发送缓冲的长度， 如果len大于s的发送缓冲区的长度，该函数返回   SOCKET_ERROR 。
    2. 如果 len 小于或者等于 sockfd 的发送缓冲区的长度，那么 send 先检查协议是否正在发送 sockfd 的发送缓冲中的数据，如果是就等待协议把数据发送完，如果协议还没有开始发送s的发送缓冲中的数据或者 sockfd 的发送缓冲中没有数据，那么 send 就比较 len 和sockfd 的发送缓冲区的剩余空间。
    3. 如果 len 大于剩余空间大小，send 就一直等待协议把 sockfd 的发送缓冲中的数据发送完；如果 len 小于剩余空间大小，send 就仅仅把 buf 中的数据 copy 到剩余空间里（注意并不是 send 把 sockfd 的发送缓冲中的数据传到连接的另一端的，而是TCP协议传的，send 仅仅是把 buf 中的数据 copy 到 sockfd 的发送缓冲区的剩余空间里）。
    4. 如果 send 函数 copy 数据成功，就返回实际 copy 的字节数，如果 send 在 copy 数据时出现错误，那么 send 就返回SOCKET_ERROR；如果 send 在等待TCP协议传送数据时网络断开的话，那么 send 函数也返回 SOCKET_ERROR 。（要注意send函数把buf中的数据成功copy到s的发送缓冲的剩余空间里后它就返回了，但是此时这些数据并不一定马上被传到连接的另一端。如果协议在后续的传送过程中出现网络错误的话，那么下一个Socket函数就会返回SOCKET_ERROR。（每一个除send外的Socket函数在执 行的最开始总要先等待套接字的发送缓冲中的数据被协议传送完毕才能继续，如果在等待时出现网络错误，那么该Socket函数就返回 SOCKET_ERROR））
  注意：在Unix系统下，如果send在等待协议传送数据时网络断开的话，调用send的进程会接收到一个SIGPIPE信号，进程对该信号的默认处理是进程终止。
  通过测试发现，异步socket的 send 函数在网络刚刚断开时还能发送返回相应的字节数，同时使用 select 检测也是可写的，但是过几秒钟之后，再 send 就会出错了，返回-1。select 也不能检测出可写了。
2. ssize_t **recv**(int sockfd, void *buf, size_t len, int flags);
  **同步编程下 send 的执行过程**（ sockfd 发送缓冲区清空才能执行 recv）：
    1. recv 先等待 sockfd 的发送缓冲中的数据被TCP协议传送完毕，如果TCP协议在传送 sockfd 的发送缓冲中的数据时出现网络错误，那么 recv 函数返回 SOCKET_ERROR 。
    2. 如果 sockfd 的发送缓冲中没有数据或者数据被TCP协议成功发送完毕后，recv 先检查套接字 sockfd 的接收缓冲区，如果 sockfd 接收缓冲区中没有数据或者TCP协议正在接收数据，那么 recv 就一直等待，直到协议把数据接收完毕。
    3. 当TCP协议把数据接收完毕，recv 函数就把 sockfd 的接收缓冲中的数据 copy 到 buf 中（注意协议接收到的数据可能大于 buf 的长度，所以在这种情况下要调用几次 recv 函数才能把 sockfd 的接收缓冲中的数据 copy 完。recv 函数仅仅是 copy 数据，真正的接收数据是TCP协议来完成的）
### 地址类
#### 抽象类
- 获得本地网络信息：int getaddrinfo(const char* hostname,const char* service,const struct addrinfo* hints,struct addrinfo** result); 
  - hostname：既可以传入**主机名称**，又可以传入字符串表示的IP地址（IPv4：点分十进制；IPv6：十六进制字符串）
  - service：既可以传入**服务名**，又可以传入字符串表示的十进制端口号。
  - hints：提示信息
  - result：链表，用以存储返回结果
### socket套接字类
#### 实现逻辑
- 创建新的 socketfd 的途径：
  1. 通过 socket() 返回一个新的 socketfd 。
  2. 通过 accept() 返回一个新的 connfd 。
  所以创建 mySocket 类的场景：
    - 先构造一个类对象，再调用 newSocket() 创建新的 socketfd 。
    - 通过 Init() 加入一个已经创建好的 connfd 。
#### 相关要点
- Nagle算法：
  - Nagle算法 主要是避免发送小的数据包，要求 TCP 连接上最多只能有一个未被确认的小分组，在该分组的确认到达之前不能发送其他的小分组。相反，TCP 收集这些少量的小分组，并在确认到来时以一个分组的方式发出去。
  ```cpp
  // 伪代码
  if there is new data to send
    if the window size >= MSS and available data is >= MSS
      send complete MSS segment now
    else
      if there is unconfirmed data still in the pipe
        enqueue data in the buffer until an acknowledge is received
      else
        send data immediately
      end if
    end if
  end if
  ```
  - 禁用Nagle算法：当你的应用不是连续请求+应答的模型的时候，而是需要实时的单项的发送数据并及时获取响应，这种case就明显不太适合Nagle算法，明显有delay的（粘包）。（ IPPROTO_TCP level中的 TCP_NODELAY 选项）
  - 关于 send 和 recv 缓冲区的定义：（会导致接收http消息被折断）
    - 见“知识碎片”中字符串的定义方法。
#### IPv4
- INADDR_ANY 宏((in_addr_t) 0x00000000)：
  在Server端bind本机IP地址和端口的时候，有些程序会使用INADDR_ANY这个地址来取代本机地址。这个宏能够让程序员在不知道本机IP地址的情况下，使用它来代表本机所有接口的IP地址。也就是说，使用这个宏作为监听地址的话，不管本机有多少个接口，socket都会监听。
- BYTE_ORDER 宏可以识别当前编译环境的字节序，可为 BIG_ENDIAN 或 LITTLE_ENDIAN
- IPv4地址从二进制转为点分十进制sylar的编程方法：
  ```cpp
    os<<((addr>>24) & 0xff)<<"."
      <<((addr>>16) & 0xff)<<"."
      <<((addr>>8)  & 0xff)<<"."
      <<( addr      & 0xff)
  ```
- 从网络字节序转为可读序列时，inet_ntoa() 是不可重入的，而 inet_ntop() 是可重入的。（不可重入：The string is returned in a statically allocated buffer, which subsequent calls will overwrite.）
  ```cpp
    sockaddr_in addr;
    const char* ip="192.168.8.106";
    inet_pton(AF_INET,ip,&addr.sin_addr);
    std::cout<<std::hex<<addr.sin_addr.s_addr<<std::endl;     // 6a08a8c0

    uint32_t mask=CreateMask<uint32_t>(24);
    // mask=ntohl(mask);
    std::cout<<std::hex<<ntohl(mask)<<std::endl;          // ffffff00

    uint32_t ans= addr.sin_addr.s_addr & (mask);
    std::cout<<"网段："<<std::hex<<ans<<std::endl;          // 8a8c0

    ans= addr.sin_addr.s_addr | (~mask);
    std::cout<<"广播："<<std::hex<<ans<<std::endl;          // ff08a8c0
  ```
### Socket_Stream 流
- TCP 分包、粘包
  - 分包：
    1. TCP自动分包：TCP是以段(Segment)为单位发送数据的,建立TCP链接后,有一个最大消息长度(MSS).如果应用层数据包超过MSS,就会把应用层数据包拆分,分成两个段来发送.这个时候接收端的应用层就要拼接这两个TCP包，才能正确处理数据。相关的,路由器有一个MTU( 最大传输单元)一般是1500字节,除去IP头部20字节,留给TCP的就只有MTU-20字节。所以一般TCP的MSS为MTU-20=1460字节，当应用层数据超过1460字节时,TCP会分多个数据包来发送。
    2. 用户自己分包：比如我们定义每一次发送的数据大小为8k(因为在真正的项目编程中基本都是要进行封装的，所以发送的大小基本固定)，那如果我们要发送一个25k的数据，我们就得分成8+8+8+1四个包发送，前三个包都是8k，最后一个包是小于8k。但容易出现的问题是：用户在应用层定义的每个分包的包头信息，在TCP协议层又被重新分包，导致实际发送到第一个报中含有第二个包的包头信息。
  - 粘包：
    1. TCP自动粘包：由于TCP数据段在发送的时候超过MSS，协议会自动的分包，所以也得把它粘起来组成一个完整的包。
    2. 用户自己粘包：把自定义分开发送的数据(8+8+8+1)重新粘在一起组成25k的原数据。
  - 如何实现粘包：
    - 先处理TCP自己分的包粘在一起，我们先将recv接受的数据appand添加到string类型的变量，然后判断string变量的长度和头部中的包大小进行比较，如果string变量的长度大于或等于包大小，说明该包已经接收完，就进行处理。如果小于则不做处理。
    - 再处理我们自己分的包，根据包头信息将数据复制到新的string变量中。如果头部的当前包序号等于总包数，说明这个包是最后一个包了，
  - 总而言之，处理分包、粘包，是根据消息头来实现的。

## 序列化和反序列化
### 定义
- 结构化数据（对象） ————> 字节流：序列化
  字节流 ————> 结构化数据（对象）：反序列化。
  简单来说，序列化就是将对象实例的状态转换为可保持或传输的格式的过程。
- 背景：
  - 在TCP的连接上，它传输数据的基本形式就是二进制流，也就是一段一段的1和0。
  - 在一般编程语言或者网络框架提供的API中，传输数据的基本形式是字节，也就是Byte。一个字节就是8个二进制位，8个Bit。
  - 二进制流和字节流本质上是一样的。对于我们编写的程序来说，它需要通过网络传输的数据是结构化的数据，比如，一条命令、一段文本或者一条消息。对应代码中，这些结构化的数据都可以用一个类或者一个结构体来表示。
- 应用场景：
  - 网络传输
  - 将结构化的数据保存在文件中（磁盘等硬件）
- 为什么需要序列化：
  1. 内存里存的东西，不通用， 不同系统， 不同语言的组织可能都是不一样的，如果直接用内存中的数据，可能会造成语言不通。只要对序列化的数据格式进行了协商，任何2个语言直接都可以进行序列化传输、接收。
  2. 一个数据结构，里面存储的数据是经过非常多其他数据结构通过非常复杂的算法生成的，如果每次生成数据结构时都要经过大量算法会有很大的时间开销。假设你确定生成数据结构的算法不会变或不常变，那么就能够通过序列化技术生成数据结构数据存储到磁盘上，下次又一次执行程序时仅仅须要从磁盘上读取该对象数据就可以，所花费时间也就读一个文件的时间。
  3. 虽然都是二进制的数据，但是序列化的二进制数据是通过一定的协议将数据字段进行拼接。
       - 第一个优势是：不同的语言都可以遵循这种协议进行解析，实现了跨语言。
       - 第二个优势是：这种数据可以直接持久化到磁盘，从磁盘读取后也可以通过这个协议解析出来。
### 实现逻辑
- 数据压缩：（全是位运算，，哭了）
  - 正数：protobuf---每个字节的高阶位为标志位，标志是否还有未表示的字节；所以只有 7 位表示数据。
  - 负数：zigzag
- 读写：针对指向内存空间的节点链表进行读写。
  - 固定长度读写：考虑字节序的转换
  - 可变长度（可压缩）读写：逐个字节的读写，不需要考虑字节序的问题了。

## TCP层
### 主要功能
- 创建监听socket，并循环 accept
- 创建 conn_socket ，启动 conn_socket 的处理函数
### 实现逻辑
#### TCP_Server抽象类
- conn_socket处理函数 handleClient 需要在子类中实现
#### echo测试程序

## HTTP层
### 协议相关知识点
- **URI**---统一资源标识符*Uniform Resource Identifier*：
  - URI 不完全等同于网址，它包含有 **URL**（统一资源定位符*Uniform Resource Locator*） 和 **URN** 两个部分。
  - 组成：
    ```
      协议名     +     主机名     +     路径     +     查询参数     +     标签
      (scheme)      (host:post)      (path)         (?query)      (#fragment)
      http://   www.baidu.com:80   /page/XXX        ?ie=utf-8&f=8
    ```
    - query查询参数：用一个“?”开始，但不包含“?”，表示对资源附加的额外要求。查询参数 query 有一套自己的格式，是多个“key=value”的字符串，这些 KV 值用字符“&”连接。
    - fragment标签（锚点）：浏览器可以在获取资源后直接跳转到它指示的位置。片段标识符仅能由浏览器这样的客户端使用，服务器是看不到的。也就是说，浏览器永远不会把带“#fragment”的 URI 发送给服务器，服务器也永远不会用这种方式去处理资源的片段。
- 报头：(每一个报头域都是由 **key+“:”+空格+value** 组成，消息报头域的名字是大小写无关的)
  - 普通报头：包含请求和响应消息都支持的头域，包含Cache-Control、Connection、Date、Pragma、Transfer-Encoding、Upgrade、Via。
  - 请求报头：包含客户端向服务器端传递请求的附加信息以及客户端自身的信息。包含Host、Accept、Accept-Charset、Accept-Encoding、Accept-Language、Authorization、User-Agent等。
  - 响应报头：用于服务器传递自身信息的响应。
  - 实体报头：用来描述消息体内容。实体报头既可用于请求也可用于响应中。包含Content-Length、Content-Language、Content-Encoding等。
### HTTP 解析
- 文本解析：ragel
- 直接复用 https://github.com/mongrel2/mongrel2/tree/master/src/http11 ，该调用实现了http解析时的有限状态机，每一部分的解析回调需要自己完成。
### HTTP Session
### HTTP Server
- 实现功能：
  - 封装下层接口（TCP_Server），重载 HandleClient() 

## 性能测试
### 工具
- AB httpd-tools 安装：http://t.zoukankan.com/chevin-p-10222681.html
### 步骤
1. 命令行：ulimit -a
  用于查看当前资源限制设置
2. cd /usr/local/httpd/bin
3. ./ab -n 1000000 -c 200 "localhost:12345/lqt"
### 测试细节
- 主机为4核，让出一个核给ab，所以服务器开3个线程比较合理
### 压测结果
```
echo_server_2 5线程：
  Server Software:        myWeb/1.0.0
  Server Hostname:        localhost
  Server Port:            12345

  Document Path:          /lqt
  Document Length:        136 bytes

  Concurrency Level:      200
  Time taken for tests:   72.149 seconds
  Complete requests:      236019
  Failed requests:        0
  Non-2xx responses:      236022
  Total transferred:      58533456 bytes
  HTML transferred:       32098992 bytes
  Requests per second:    3271.29 [#/sec] (mean)
  Time per request:       61.138 [ms] (mean)
  Time per request:       0.306 [ms] (mean, across all concurrent requests)
  Transfer rate:          792.27 [Kbytes/sec] received

  Connection Times (ms)
                min  mean[+/-sd] median   max
  Connect:        0    0   0.4      0      13
  Processing:     2   61  10.0     59     109
  Waiting:        1   60   9.9     58     109
  Total:          8   61  10.0     59     109

  Percentage of the requests served within a certain time (ms)
    50%     59
    66%     61
    75%     64
    80%     68
    90%     79
    95%     82
    98%     85
    99%     87
    100%    109 (longest request)

===================================================================

test_http_server 2线程：
  Server Software:        myWeb/1.0.0
  Server Hostname:        192.168.8.106
  Server Port:            12345

  Document Path:          /lqt
  Document Length:        136 bytes

  Concurrency Level:      200
  Time taken for tests:   120.631 seconds
  Complete requests:      389340
  Failed requests:        0
  Non-2xx responses:      389340
  Total transferred:      96556320 bytes
  HTML transferred:       52950240 bytes
  Requests per second:    3227.52 [#/sec] (mean)
  Time per request:       61.967 [ms] (mean)
  Time per request:       0.310 [ms] (mean, across all concurrent requests)
  Transfer rate:          781.66 [Kbytes/sec] received

  Connection Times (ms)
                min  mean[+/-sd] median   max
  Connect:        0    0   0.2      0      12
  Processing:     2   62  10.0     63     100
  Waiting:        1   62  10.0     63     100
  Total:         12   62  10.0     63     100

  Percentage of the requests served within a certain time (ms)
    50%     63
    66%     67
    75%     68
    80%     69
    90%     73
    95%     78
    98%     82
    99%     84
    100%    100 (longest request)

===================================================================

test_http_server 1线程：
  Server Software:        myWeb/1.0.0
  Server Hostname:        192.168.8.106
  Server Port:            12345

  Document Path:          /lqt
  Document Length:        136 bytes

  Concurrency Level:      200
  Time taken for tests:   105.629 seconds
  Complete requests:      319036
  Failed requests:        0
  Non-2xx responses:      319036
  Total transferred:      79120928 bytes
  HTML transferred:       43388896 bytes
  Requests per second:    3020.35 [#/sec] (mean)
  Time per request:       66.217 [ms] (mean)
  Time per request:       0.331 [ms] (mean, across all concurrent requests)
  Transfer rate:          731.49 [Kbytes/sec] received

  Connection Times (ms)
                min  mean[+/-sd] median   max
  Connect:        0    0   0.2      0      11
  Processing:     4   66   7.1     66     106
  Waiting:        2   66   7.1     66     106
  Total:         13   66   7.1     66     106

  Percentage of the requests served within a certain time (ms)
    50%     66
    66%     68
    75%     70
    80%     71
    90%     76
    95%     81
    98%     83
    99%     84
    100%    106 (longest request)

===================================================================

test_http_server 1线程（不打日志）：
  Server Software:        myWeb/1.0.0
  Server Hostname:        192.168.8.106
  Server Port:            12345

  Document Path:          /lqt
  Document Length:        136 bytes

  Concurrency Level:      200
  Time taken for tests:   94.270 seconds
  Complete requests:      1000000
  Failed requests:        0
  Non-2xx responses:      1000000
  Total transferred:      248000000 bytes
  HTML transferred:       136000000 bytes
  Requests per second:    10607.84 [#/sec] (mean)
  Time per request:       18.854 [ms] (mean)
  Time per request:       0.094 [ms] (mean, across all concurrent requests)
  Transfer rate:          2569.09 [Kbytes/sec] received

  Connection Times (ms)
                min  mean[+/-sd] median   max
  Connect:        0    0   0.1      0      11
  Processing:     4   19   0.4     19      28
  Waiting:        1   19   0.4     19      28
  Total:         13   19   0.4     19      28

  Percentage of the requests served within a certain time (ms)
    50%     19
    66%     19
    75%     19
    80%     19
    90%     19
    95%     19
    98%     20
    99%     20
    100%     28 (longest request)

===================================================================

test_http_server 3线程（不打日志）：
  Server Software:        myWeb/1.0.0
  Server Hostname:        192.168.8.106
  Server Port:            12345

  Document Path:          /lqt
  Document Length:        136 bytes

  Concurrency Level:      200
  Time taken for tests:   79.333 seconds
  Complete requests:      1000000
  Failed requests:        0
  Non-2xx responses:      1000000
  Total transferred:      248000000 bytes
  HTML transferred:       136000000 bytes
  Requests per second:    12605.17 [#/sec] (mean)
  Time per request:       15.867 [ms] (mean)
  Time per request:       0.079 [ms] (mean, across all concurrent requests)
  Transfer rate:          3052.81 [Kbytes/sec] received

  Connection Times (ms)
                min  mean[+/-sd] median   max
  Connect:        0    7   0.9      7      18
  Processing:     3    9   1.4      9      20
  Waiting:        1    7   1.6      8      19
  Total:          9   16   1.0     16      27

  Percentage of the requests served within a certain time (ms)
    50%     16
    66%     16
    75%     16
    80%     16
    90%     17
    95%     18
    98%     19
    99%     20
    100%     27 (longest request)

===================================================================

test_http_server 3线程（不打日志）：
  Server Software:        myWeb_TCP/1.0.0
  Server Hostname:        192.168.8.106
  Server Port:            12345

  Document Path:          /lqt/x
  Document Length:        123 bytes

  Concurrency Level:      100
  Time taken for tests:   11.844 seconds
  Complete requests:      148860
  Failed requests:        0
  Total transferred:      30814848 bytes
  HTML transferred:       18310272 bytes
  Requests per second:    12568.40 [#/sec] (mean)
  Time per request:       7.956 [ms] (mean)
  Time per request:       0.080 [ms] (mean, across all concurrent requests)
  Transfer rate:          2540.75 [Kbytes/sec] received

  Connection Times (ms)
                min  mean[+/-sd] median   max
  Connect:        0    3   0.5      3       7
  Processing:     3    5   0.7      5      10
  Waiting:        1    4   0.8      4       8
  Total:          7    8   0.6      8      13

  Percentage of the requests served within a certain time (ms)
    50%      8
    66%      8
    75%      8
    80%      8
    90%      8
    95%      9
    98%     10
    99%     10
    100%     13 (longest request)

```

## 知识碎片
- &emsp;在 Linux 下的异步 I/O 是不完善的， aio 系列函数是由 POSIX 定义的异步操作接口，不是真正的操作系统级别支持的，而是在用户空间模拟出来的异步，并且仅仅支持基于本地文件的 aio 异步操作，网络编程中的 socket 是不支持的，这也使得基于 Linux 的高性能网络程序都是使用 Reactor 方案。
&emsp;而 Windows 里实现了一套完整的支持 socket 的异步编程接口，这套接口就是 IOCP，是由操作系统级别实现的异步 I/O，真正意义上异步 I/O，因此在 Windows 里实现高性能网络程序可以使用效率更高的 Proactor 方案。
- 同步和异步 有两种概念：
  1. I/O处理：
     - 同步：需要程序自己调用系统调用，将内核态数据拷贝到用户态缓冲区。则进程阻塞态中需要等待 **内核数据准备好** 和 **数据从内核态拷贝到用户态** 这两个过程。
     - 异步：以上两个过程都不需要等待，数据准备好后，由内核自动将数据拷贝到用户态。
  2. 编程方式：
     - 同步：程序调用某方法，只有该方法执行完全部操作后，程序才能继续执行。
     - 异步：在上述调用的方法执行完之前，该方法就返回了。
         使用异步编程的逻辑：
            消除IO阻塞带来的性能损失————>IO多路复用、多线程（线程池）
            此时又想消除频繁上下文切换带来的性能损失————>异步编程
- 宏可以帮助避免重复引入头文件。是全局的，在编译期间展开直接替换，所以不受明明空间约束。
  https://zhuanlan.zhihu.com/p/144762106
- 野指针：指向不可用的内存区域的指针，是指向“垃圾”的指针。造成野指针的原因：
  - 指针变量没有被初始化。
  - 指针指向的内存被释放了，而指针本身未被置位 nullptr。（所以在 free() 一个指针的时候，要将指针变量设置为 nullptr 。）
  - 指针超过了变量的作用范围。（局部变量的作用范围虽然已经结束，内存已经被释放，然而地址值仍是可用的，不过随时都可能被内存管理分配给其他变量。）
- 一些与字符串相关的函数，内部可能封装了 malloc() ，所以常与 free() 搭配。
- :: 前没有命名空间，表示系统相关。
- ***关于 assert()*** :
  - 在添加 assert() 调试的时候，发现在调用宏 MYWEB_ASSERT(x) 的时候，发现无法通过 FilelogAppender 向文件写入日志（见 test_assert.cc ）。
  - 通过把宏定义里 assert() 函数注释掉，发现回复正常。可以推断是 assert() 函数搞鬼。
  - 程序中主函数和调用函数分别增加 ofstream 对象实验，发现仍然不行。
  - 由此推断，assert() 执行后会 abort 当前程序，所以**会取消该程序中已执行的文件写操作**。
- function<> ：
  - 可调用类型包括：
    - 传统C函数；
    - 类中的成员函数；
    - 函数对象(类中重载了“()”运算符);
    - lambda表达式。
  - 使用函数指针需要对不同的可调用类型(callable object)进行单独声明，将函数指针作为函数参数只能接收某种具体类型，非常不灵活。而function<>满足了兼容所有可调用类型的需求，提高了程序设计的灵活性
- std::atomic<> 是不可复制的，所以**只能用列表初始化**，且一定要初始化。
- 类的非静态成员函数可以访问静态成员函数，反之不可以（除非通过对象访问）。但是静态成员函数可以通过类**私有的构造函数**创建对象（见Fiber类）。
- 类的对象A和B，B可以通过类的成员函数访问对象A的私有变量。————类的成员函数是不占用对象的空间的，成员函数是属于类的，而不是属于变量，这也就是说，私有变量可以直接被类的成员函数访问，无论是通过哪个对象调用这个成员函数。
- 类继承 enable_shared_from_this<> ，可以通过 shared_from_this() 返回类的 this 的智能指针：
  - 把当前类对象作为参数传给其他函数时，为什么要传递 share_ptr 呢？直接传递 this 指针不可以吗？
      一个裸指针传递给调用者，谁也不知道调用者会干什么。假如调用者 delete 了该对象，而传出的指针此时还指向该对象，会产生野指针。
  - 通过 share_ptr<this> 传递可以吗?
      不可以，这样会造成2个非共享的share_ptr指向一个对象，最后造成2次析构该对象。
  - 所以通过 shared_from_this() 传递的智能指针与外界指向该对象的智能指针是**共享**的。
- 结构体 stuct 要重载构造函数的话，一定要给出默认构造函数。
- 结构体可以用参数列表初始化，参数列表中顺序写入成员变量的初始值。
- int ftcnl(int fd,int cmd,...) cmd 参数表示执行何种类型的操作。其中有 F_SETFD（设置fd的标志）和 F_SETFL（设置fd的状态标志）。如何理解 **fd的标志** 和 **fd的状态标志** ：
  - fd的标志：是文件描述符本身的状态标志
  - fd的状态标志：是文件描述符所指向的文件的状态标志
- vector 的 reserve() 和 resize():
  - reserve(n) : 为 vector 对象分配一个大小为 n 的连续内存空间，即 有效容量+冗余容量=n 。如果原本容量大于 n ，则什么也不做。
  - resize(n,const T& c=T()) : 为 vector 对象分配一个固定大小为 n 的连续空间，且 有效容量=n 。
    - 如果原本容量小于 n ，则用默认值 c 创建扩容
    - 如果原本容量大于 n ，则从容器末尾删除元素直到有效容量为 n 。
- shared_ptr 和 new 结合使用：
  - shared_ptr<T> p(q,d) ：p 接管了内置指针 q 所指向的对象的所有权。q 必须能转换为类型 T* ，p 将使用可调用对象 d 来代替delete 。
- std::set<T,C>/std::map<T,C> 如果传入自定义类型 T ，则需要给 set/map 传入比较方法（或重载 operator<() ，或添加仿函数 C （要重载 bool opertor()(T left,T right) ））。如果 T 是指针，不传比较方法的话，则默认以地址比较。
- Linux 时钟源：
  - RTC(Real Time Clock)：它是PC主机板上的一块芯片，它靠电池供电，即使系统断电，也可以维持日期和时间。由于它独立于操作系统，所以也被称为硬件时钟，它为整个计算机提供一个计时标准，是最原始最底层的时钟数据。通过作用于/dev/rtc设备文件，也允许进程对RTC编程。通过执行/sbin/clock系统程序，系统管理员可以配置时钟。
  - OS时钟：产生于PC主板上的定时/计数芯片，由操作系统控制这个芯片的工作，OS时钟的基本单位就是该芯片的计数周期。在开机时操作系统取得RTC中的时间数据来初始化OS时钟，完全由操作系统控制，所以也被称为软时钟或系统时钟。操作系统通过OS时钟提供给应用程序所有和时间有关的服务。
- 如何在 main 函数之前执行一段程序：
  - 在 main 之前通常要对全局变量或静态变量初始化，如果这个变量实例是个类，在进入 main 函数之前一定会先执行它的构造函数。利用这个特性就可以实现在 main 函数之前就先执行其他程序。（单例）
- hook.cc 中 (uint64_t)-1 即为 2^64-1 
  - 当具有整数类型的值转换为除_Bool之外的另一个整数类型时，如果该值可以由新类型表示，则它将保持不变。
  - 否则，如果新类型是无符号的，则通过重复加或减一个可以在新类型中表示的最大值来转换该值，直到该值在新类型的范围内。 
  - 否则，新类型已签名且值无法在其中表示; 结果是实现定义的，或者引发实现定义的信号。
- 模板元编程时类型选择的小工具：
  ```cpp
  /* 只有bool为true，type才有定义
   * 可用于偏特化模板类型*/
  template <bool, typename T=void>
  struct enable_if {
  };
  template <typename T>
  struct enable_if<true, T> {
    using type = T;
  };
  ```
- 定义字符串的方式：
  ```cpp
  char* addr;                   // 错误
  char addr[INET_ADDRSTRLEN];   // 对
  const char* addr;             // 错，底层常量,指向变量只读
  ```
- 在类中调用与类成员函数同名的系统调用时，函数前面要加 “::” 以表示系统函数
- C++11 新语法：枚举类 enum class 枚举类型名:底层类型{枚举值列表};
  - 相比普通枚举的优势：
     1. 强作用域，其作用域限制在枚举类中。而普通枚举是全局的，枚举值会与其他枚举定义冲突
     2. 转换限制，枚举类对象不可以与整形隐式地互相转换
     3. 可以指定底层类型

## Question
1. 如何保证STL容器的迭代器在多线程中不会失效？如 Logger 中的 appenders 成员变量，在 log() 和 addAppender() 都会对appenders进行修改，这时 log() 中的迭代器是否会失效？
2. 在 Logger 类中加入互斥锁后，在 LogManager::getLogger() 中使用 make_shared 创建 Logger ，显示 “error: use of deleted function ‘myWeb::Logger::Logger(myWeb::Logger&&)’” ？
   - A: 互斥锁是不可赋值、不可移动构造、不可赋值构造的，作为 Logger 的成员变量，向上影响到 Logger 也变为不可赋值、不可移动构造、不可赋值构造的。而 make_shared 的可变参数传入的是**移动构造的对象**，所以发生错误。
   - S: 将 make_shared 改为 shared_ptr<> obj() 直接构造智能指针。
3. 在 mthread.cc 中静态全局变量 t_thread 是指针类型，而类中又定义了智能指针类型，在 test_thread.cc 中使用了智能指针，会不会造成智能指针和普通指针的混用？——————猜测 Thread::ptr 是危险的
   - 智能指针和普通指针混用：
      在一个函数中使用智能指针，在用的时候会有引用计数增加，这个时候指针指向的内存是有值的。接着将一个正常指针指向这块内存区域，都能正常访问，但是他没有通知智能指针增加引用计数，这就埋下伏笔！如果智能指针不再被使用，（出了作用域等）引用计数减完，然后C++会自动删除指针所指向的内存区域，并释放！这时，正常指针并不知道自己所指向的区域已经被删除了，就变成了野指针。
4. std::bind() 绑定类实例的成员函数时，参数列表第一个参数应该是该实例。
  ```cpp
    // schedule() 需要往指定 IOManager 对象中添加执行函数，作为类的成员函数需要指定实例对象才能调用
    iomanager->addTimer((uint64_t)seconds*1000,std::bind(
              (void(myWeb::Scheduler::*)(myWeb::Fiber::ptr,pid_t))&myWeb::IOManager::schedule,
              iomanager,fiber,-1));       // hook.cc 中一例
  ```
5. 关于 std::string::c_str() 返回的常量字符串指针的生命周期的问题：（ address.cc ）
  - Q：为何前后两次调用，返回的指针可以指向同一地址？
  - A：从 c_str() 获得的指针可能因以下原因而失效：（在下一次调用前，string 对象销毁或修改会导致指针失效）
    1. 将对字符串的非常量引用传递给任何标准库函数
    2. 在string上调用非常量成员函数，但不包括operator[]，at()，front()，back()，begin()，rbegin()和end()。
6. 段错误 Debug 可真tm难
7. ```cpp
  // 出现错误：comparison with string literal results in unspecified behavior
  if(typeid(type).name()=="float" || typeid(type).name()=="double")   
   ```
  原因：在 C++ 中，== 仅在内部为原始类型（如int、char、bool…）实现，而 const char* 不是原始类型，因此const char* 和字符串会作为两个char* 比较，即两个指针的比较，而这是两个不同的指针，“connection”== nextElement->Name()不可能为真。
8. HttpSession 中，sendResponse 在与 recvRequest 同一个schedule（协程）中执行时，出现死循环，所在线程无法切换其他协程（无法 yieldtoHold ），抢占该线程无法被调度。
  原因：一个循环写错了
  心得：准确描述问题后应该认真回溯代码，逻辑没问题的话就要仔细检查每一处细节。


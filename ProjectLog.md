## log
1. Logger是最终的日志生成接口，LogEvent为日志事件，包含所有信息。但是LogEvent中包含了Logger的智能指针，为了防止循环引用就不能再在Logger中添加LogEvent智能指针。但是为满足：
- `logger->log(myWeb::LogLevel::WARN,event);`中需要event指针作为参数。
- 使用方便，最好是一个接口。
- 可以调用LogEvent::getSS()作为日志内容的输出流。
所以使用“宏（便于缩减代码）+LogEventWrap类”
2. 类的值成员都应该是 'private' 的，所以都应该通过函数接口来访问。
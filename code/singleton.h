#ifndef __MYWEB_SINGLETON_H__
#define __MYWEB_SINGLETON_H__

namespace myWeb{
    
// 懒汉式单例模式
template<typename T>
class Singleton{
public:
    static T* getInstance(){
        static T obj;       // 应用了局部静态变量的性质
        return &obj;
    }
};

}

#endif
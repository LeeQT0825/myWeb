#ifndef __MYWEB_CONFIG_H__
#define __MYWEB_CONFIG_H__

#include <string>
#include <memory>
#include <stdio.h>
#include <exception>
#include <boost/lexical_cast.hpp>
#include <unordered_map>
#include "log.h"
#include "singleton.h"

namespace myWeb{
// 配置变量基类 (封装一些共用的属性，所以无需是模板类)
class ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;

    ConfigVarBase(const std::string& name,const std::string& discription=""):
            m_name(name),
            m_discription(discription){
        std::transform(m_name.begin(),m_name.end(),m_name.begin(),::tolower);
    }
    virtual ~ConfigVarBase(){}

    // 转换到字符串
    virtual std::string ToString()=0;
    // 解析：从字符串初始化值
    virtual bool FromString(const std::string& val)=0;

    // 返回参数名称
    const std::string& getName() const{
        return m_name;
    }
    // 返回参数描述
    const std::string& getDiscription() const{
        return m_discription;
    }
    // 返回配置变量类型
    virtual std::string getTypeName() const=0;
protected:
    // 配置参数的名称
    std::string m_name;
    // 配置参数的描述
    std::string m_discription;
};

// 具体实现类(模板类)
template<typename T>
class ConfigVar:public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;

    // 这里有个小细节：默认参数要放到后面！！！
    ConfigVar(const std::string& name,const T& val,const std::string& discription=""):
            ConfigVarBase(name,discription),
            m_val(val){}

    // 将参数值转换为YAML String（为后续输出yaml配置文档作铺垫）
    std::string ToString() override {
        try{
            return boost::lexical_cast<std::string>(m_val);     // 用于基础类型的转化
        }catch(std::exception& e){
            INLOG_ERROR(MYWEB_ROOT_LOG)<<"ConfigVar::ToString() exception"
                <<e.what()<<" convert: "<< typeid(m_val).name() << "to string";
        }
        return "";
    }
    // 读取yaml文件，将YAML String转换为本来的类型
    bool FromString(const std::string& val) override {
        try{
            m_val=boost::lexical_cast<T>(val);
        }catch(std::exception& e){
            INLOG_ERROR(MYWEB_ROOT_LOG)<<"ConfigVar::FromString() exception"
                <<e.what()<<" convert: string to"<< typeid(m_val).name();
        }
        return false;
    }
    // 获取配置变量值
    const T& getVal() const {
        return m_val;
    }
    // 设置配置变量值
    void setVal(const T& val){
        m_val=val;
    }
    // 获取配置变量类型
    std::string getTypeName() const override{
        return typeid(m_val).name();
    }

    // TODO：配置变量的回调函数

private:
    // 配置变量值
    T m_val;
};

// 配置管理类，单例模式
class Config{
public:
    typedef std::unordered_map<std::string,ConfigVarBase::ptr> ConfigVarMap;

    // 根据配置变量查询，返回配置变量值
    // 重载一：有就返回，没有就用val创建
    template <typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,const T& val,const std::string& discription=""){
        auto iter=getConfigVarMap()->find(name);
        if(iter!=getConfigVarMap()->end()){
            if(iter->second){
                INLOG_INFO(MYWEB_ROOT_LOG)<<"Lookup name: "<<name<<" exists"<<std::endl;
                return std::dynamic_pointer_cast<ConfigVar<T> >(iter->second);      // 要返回子类类型，必须将父类指针强制转化为子类指针
            }
        }

        // 检验name是否合法
        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._1234567890")!=std::string::npos){
            INLOG_ERROR(MYWEB_ROOT_LOG)<<"Lookup name: "<<name<<" is ivalid"<<std::endl;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr arg(new ConfigVar<T>(name,val,discription));
        getConfigVarMap()->at(name)=arg;
        return arg;
    }
    // 重载二：有就返回，无就返回空指针
    template <typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name){
        auto iter=getConfigVarMap()->find(name);
        if(iter!=getConfigVarMap()->end()){
            return std::dynamic_pointer_cast<ConfigVar<T> >(iter->second);      // 要返回子类类型，必须将父类指针强制转化为子类指针
        }
        return nullptr; 
    }

private:
    // 单例模式的配置管理器
    static ConfigVarMap* getConfigVarMap(){
        return Singleton<ConfigVarMap>::getInstance();
    }
};


}
#endif
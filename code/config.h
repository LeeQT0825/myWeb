#ifndef __MYWEB_CONFIG_H__
#define __MYWEB_CONFIG_H__

#include <string>
#include <vector>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include <stdio.h>
#include <exception>
#include <boost/lexical_cast.hpp>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
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

// 通用类型转化类
// F: FromType  T: ToType
template<typename F,typename T>
class LexicalCast{
public:
    T operator()(const F& fval){
        return boost::lexical_cast<T>(fval);
    }
};

// TODO: 具体复杂类型的片特化

//From:string  To:vector
template<typename T>
class LexicalCast<std::string,std::vector<T> >{
public:
    std::vector<T> operator()(const std::string& fval){
        YAML::Node node=YAML::Load(fval);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i=0;i<node.size();++i){
            ss.str("");     // 清空ss
            ss<<node[i];
            vec.push_back(LexicalCast<std::string,T>()(ss.str()));      // 先实例化，再调用重载调用符
        }
        return vec;
    }
};
//From:vector  To:string
template<typename T>
class LexicalCast<std::vector<T>,std::string>{
public:
    std::string operator()(const std::vector<T>& fval){
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i:fval){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};
//From:string  To:list
template<typename T>
class LexicalCast<std::string,std::list<T> >{
public:
    std::list<T> operator()(const std::string& fval){
        YAML::Node node=YAML::Load(fval);
        typename std::list<T> lst;
        std::stringstream ss;
        for(size_t i=0;i<node.size();++i){
            ss.str("");     // 清空ss
            ss<<node[i];
            lst.push_back(LexicalCast<std::string,T>()(ss.str()));      // 先实例化，再调用重载调用符
        }
        return lst;
    }
};
//From:list  To:string
template<typename T>
class LexicalCast<std::list<T>,std::string>{
public:
    std::string operator()(const std::list<T>& fval){
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i:fval){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};
//From:string  To:set
template<typename T>
class LexicalCast<std::string,std::set<T> >{
public:
    std::set<T> operator()(const std::string& fval){
        YAML::Node node=YAML::Load(fval);
        typename std::set<T> st;
        std::stringstream ss;
        for(size_t i=0;i<node.size();++i){
            ss.str("");     // 清空ss
            ss<<node[i];
            st.insert(LexicalCast<std::string,T>()(ss.str()));      // 先实例化，再调用重载调用符
        }
        return st;
    }
};
//From:set  To:string
template<typename T>
class LexicalCast<std::set<T>,std::string>{
public:
    std::string operator()(const std::set<T>& fval){
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i:fval){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};
//From:string  To:unordered_set
template<typename T>
class LexicalCast<std::string,std::unordered_set<T> >{
public:
    std::unordered_set<T> operator()(const std::string& fval){
        YAML::Node node=YAML::Load(fval);
        typename std::unordered_set<T> st;
        std::stringstream ss;
        for(size_t i=0;i<node.size();++i){
            ss.str("");     // 清空ss
            ss<<node[i];
            st.insert(LexicalCast<std::string,T>()(ss.str()));      // 先实例化，再调用重载调用符
        }
        return st;
    }
};
//From:unordered_set  To:string
template<typename T>
class LexicalCast<std::unordered_set<T>,std::string>{
public:
    std::string operator()(const std::unordered_set<T>& fval){
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i:fval){
            node.push_back(YAML::Load(LexicalCast<T,std::string>()(i)));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};
//From:string  To:map
template<typename T>
class LexicalCast<std::string,std::map<std::string,T> >{
public:
    std::map<std::string,T> operator()(const std::string& fval){
        YAML::Node node=YAML::Load(fval);
        typename std::map<std::string,T> mp;
        std::stringstream ss;
        for(auto iter=node.begin();iter!=node.end();++iter){
            ss.str("");     // 清空ss
            ss<<iter->second;
            mp.insert(std::make_pair(iter->first.Scalar(),LexicalCast<std::string,T>()(ss.str())));      // 先实例化，再调用重载调用符
        }
        return mp;
    }
};
//From:map  To:string
template<typename T>
class LexicalCast<std::map<std::string,T>,std::string>{
public:
    std::string operator()(const std::map<std::string,T>& fval){
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i:fval){
            node[i.first]=YAML::Load(LexicalCast<T,std::string>()(i.second));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};
//From:string  To:unordered_map
template<typename T>
class LexicalCast<std::string,std::unordered_map<std::string,T> >{
public:
    std::unordered_map<std::string,T> operator()(const std::string& fval){
        YAML::Node node=YAML::Load(fval);
        typename std::unordered_map<std::string,T> mp;
        std::stringstream ss;
        for(auto iter=node.begin();iter!=node.end();++iter){
            ss.str("");     // 清空ss
            ss<<iter->second;
            mp.insert(std::make_pair(iter->first.Scalar(),LexicalCast<std::string,T>()(ss.str())));      // 先实例化，再调用重载调用符
        }
        return mp;
    }
};
//From:unordered_map  To:string
template<typename T>
class LexicalCast<std::unordered_map<std::string,T>,std::string>{
public:
    std::string operator()(const std::unordered_map<std::string,T>& fval){
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i:fval){
            node[i.first]=YAML::Load(LexicalCast<T,std::string>()(i.second));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};


// 具体实现类(模板类)
template<typename T>
class ConfigVar:public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T& oldval,const T& newval)> on_change_cb;     // 回调接口

    // 这里有个小细节：默认参数要放到后面！！！
    ConfigVar(const std::string& name,const T& val,const std::string& discription=""):
            ConfigVarBase(name,discription),
            m_val(val){}

    // 获取配置变量值
    const T& getVal() const {
        return m_val;
    }
    // 设置配置变量值
    void setVal(const T& val){
        if(val==m_val)  return;
        for(auto i:m_cbs){
            i.second(m_val,val);
        }
        m_val=val; 
    }
    // 获取配置变量类型
    std::string getTypeName() const override{
        return typeid(m_val).name();
    }

    // 将参数值转换为YAML String（为后续输出yaml配置文档作铺垫）
    std::string ToString() override {
        try{
            return LexicalCast<T,std::string>()(m_val);     // 用于基础类型的转化
        }catch(std::exception& e){
            INLOG_ERROR(MYWEB_ROOT_LOG)<<"ConfigVar::ToString() exception"
                <<e.what()<<" convert: "<< typeid(m_val).name() << "to string";
        }
        return "";
    }
    // 读取yaml文件，将YAML String转换为本来的类型
    bool FromString(const std::string& val) override {
        try{
            setVal(LexicalCast<std::string,T>()(val)); 
        }catch(std::exception& e){ 
            INLOG_ERROR(MYWEB_ROOT_LOG)<<"ConfigVar::FromString() exception"
                <<e.what()<<" convert: string to"<< typeid(m_val).name();
        }
        return false;
    }

    // TODO：配置变量的回调函数
    void addListener(uint64_t key,on_change_cb cb){
        m_cbs[key]=cb;
    }
    void delListener(uint64_t key){
        m_cbs.erase(key);
    }
    on_change_cb getListener(uint64_t key){
        auto iter=m_cbs.find(key);
        return iter==m_cbs.end()?nullptr:iter->second;
    }
    void clearCallBacks(){
        m_cbs.clear();
    }
private:
    // 配置变量值
    T m_val;
    // 变更回调函数组
    std::map<uint64_t,on_change_cb> m_cbs;
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
        getConfigVarMap()->insert({name,arg});
        return arg;
    }
    // 重载二：有就返回，无就返回空指针
    static ConfigVarBase::ptr Lookup(const std::string& name){
        auto iter=getConfigVarMap()->find(name);
        if(iter!=getConfigVarMap()->end()){
            return iter->second;      // 要返回子类类型，必须将父类指针强制转化为子类指针
        }
        return nullptr; 
    }

    // 加载配置文件
    static void LoadFromYaml(const std::string& filename);

private:
    // 单例模式的配置管理器
    static ConfigVarMap* getConfigVarMap(){
        return Singleton<ConfigVarMap>::getInstance();
    }

};


}
#endif
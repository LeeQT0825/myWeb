#include "config.h"
#include <list>
#include <sstream>

namespace myWeb{
    
static Logger::ptr config_logger(MYWEB_NAMED_LOG("config_log"));

// 根据层级关系转换为相应的数据结构
// 为什么没有Sequence的转换？？？
static void ListAllMember(const std::string& prefix,const YAML::Node& node,std::list<std::pair<std::string,const YAML::Node> >& output){
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._1234567890")!=std::string::npos){
        INLOG_ERROR(MYWEB_ROOT_LOG)<<"Config invalid name: " << prefix << " : " << node;
        return;
    }
    output.push_back(std::make_pair(prefix,node));
    if(node.IsMap()){
        for(auto iter=node.begin();iter!=node.end();++iter){
            ListAllMember(prefix.empty()?iter->first.Scalar():prefix+"."+iter->first.Scalar(),
                            iter->second,output);
        }
    }
}

void Config::LoadFromYaml(const std::string& filename){
    YAML::Node rootNode=YAML::LoadFile(filename);
    std::list<std::pair<std::string,const YAML::Node> > config;
    ListAllMember("",rootNode,config);      // 初始key为空
    for(auto& p:config){
        if(p.first.empty())     continue;
        std::transform(p.first.begin(),p.first.end(),p.first.begin(),::tolower);
        // 修改配置
        ConfigVarBase::ptr cval=Lookup(p.first);
        if(cval){
            if(p.second.IsScalar()){
                cval->FromString(p.second.Scalar());
            }else{
                std::stringstream ss;
                ss<<p.second;
                cval->FromString(ss.str());
            }
        }
    }

}

}
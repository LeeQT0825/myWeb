#include "http.h"

namespace myWeb{
namespace http{

Http_Method Str2HttpMethod(const std::string& str){
#define XX(num,name,string) \
    if(strcmp(str.c_str(),#string)==0){ \
        return Http_Method::name; \
    }

    HTTP_METHOD_MAP(XX);

#undef XX
    return Http_Method::INVALID_METHOD;
}
Http_Method Chars2HttpMethod(const char* chr){
#define XX(num,name,string) \
    if(strncmp(chr,#string,strlen(#string))==0){ \
        return Http_Method::name; \
    }

    HTTP_METHOD_MAP(XX);
    
#undef XX
    return Http_Method::INVALID_METHOD;
}
std::string HttpMethod2String(Http_Method meth){
    // 枚举类不可以隐式转换为普通类型
    switch (meth)
    {
    #define XX(num,name,string) \
        case Http_Method::name: \
            return #name;

        HTTP_METHOD_MAP(XX);

    #undef XX
        default:
            return "<unknow>";
    }
    return "<unknow>";
}
std::string HttpStatus2String(Http_Status stat){
    switch (stat)
    {
    #define XX(num,name,string) \
        case Http_Status::name: \
            return #name;

        HTTP_STATUS_MAP(XX);

    #undef XX
        default:
            return "<unknow>";
    }
    return "<unknow>";
}

bool CaseInsensitiveLess::operator()(const std::string& left,const std::string& right) const{
    return strcasecmp(left.c_str(),right.c_str())<0;
}

HttpRequest::HttpRequest(uint8_t version,bool keepalive)
                :m_method(Http_Method::GET)
                ,m_version(version)
                ,m_keepalive(keepalive)
                ,m_path("/"){
    setHeaders("connection",(m_keepalive?"keepalive":"close"));
}

std::string HttpRequest::getHeaders(const std::string& key,const std::string& def) const {
    auto iter=m_headers.find(key);
    if(iter==m_headers.end()){
        return def;
    }
    return iter->second;
}
std::string HttpRequest::getParams(const std::string& key,const std::string& def) const {
    auto iter=m_params.find(key);
    if(iter==m_params.end()){
        return def;
    }
    return iter->second;
}
std::string HttpRequest::getCookies(const std::string& key,const std::string& def) const {
    auto iter=m_cookies.find(key);
    if(iter==m_cookies.end()){
        return def;
    }
    return iter->second;
}
void HttpRequest::setHeaders(const std::string& key,const std::string& val){
    m_headers[key]=val;
}
void HttpRequest::setParams(const std::string& key,const std::string& val){
    m_params[key]=val;
}
void HttpRequest::setCookies(const std::string& key,const std::string& val){
    m_cookies[key]=val;
}
void HttpRequest::delHeaders(const std::string& key){
    m_headers.erase(key);
}
void HttpRequest::delParams(const std::string& key){
    m_params.erase(key);
}
void HttpRequest::delCookies(const std::string& key){
    m_cookies.erase(key);
}

std::ostream& HttpRequest::dump(std::ostream& os) const {
    /*  GET /uri HTTP/version
        HOST: www.baidu.com
        ----------------------  
    */
    os<<HttpMethod2String(m_method)<<" "
                                <<m_path<<(m_query.empty()?"":"?")<<m_query<<(m_fragment.empty()?"":"#")<<m_fragment
                                <<" HTTP/"<<(uint32_t)(m_version>>4)<<"."<<(uint32_t)(m_version & 0xf)
                                <<"\r\n";
    for(auto& i:m_headers){
        os<<i.first<<": "<<i.second<<"\r\n";
    }
    if(!m_body.empty()){
        os<<"content-length: "<<m_body.size()<<"\r\n\r\n";
        os<<m_body;
    }else{
        os<<"\r\n";
    }
    return os;
}



}
}
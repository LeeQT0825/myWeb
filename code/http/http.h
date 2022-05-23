#ifndef __MYWEB_HTTP_H__
#define __MYWEB_HTTP_H__

#include <memory>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>

namespace myWeb{

namespace http{

/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       

// HTTP 方法
enum class Http_Method
{
#define XX(num, name, string) name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};


/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) 

// HTTP 状态码
enum class Http_Status
{
#define XX(num, name, string) name = num,
    HTTP_STATUS_MAP(XX)
#undef XX
};

// String 表述的方法转为Http方法（区分大小写）
Http_Method Str2HttpMethod(const std::string& str);
// 字符串表述的方法转为Http方法（区分大小写）
Http_Method Chars2HttpMethod(const char* chr);
// Http方法转为字符串描述（区分大小写）
std::string HttpMethod2String(Http_Method meth);
// Http状态转为字符串描述（区分大小写）
std::string HttpStatus2String(Http_Status stat);

// 忽略大小写的比较仿函数
struct CaseInsensitiveLess{
    bool operator()(const std::string& left,const std::string& right) const ;
};

// 获取map中的key值，并存储到 val 中，返回bool
template<typename T,typename MapType>
bool checkAs(const MapType& mp,const std::string& key,T& val,const T& def=T()){
    auto iter=mp.find(key);
    if(iter==mp.end()){
        val=def;
        return false;
    }

    try{
        val=boost::lexical_cast<T>(iter->second);
        return true;
    }catch(...){
        val=def;
        return false;
    }
}

// 获取map中的key值，并转成相应的类型
template<typename T,typename MapType>
T getAs(const MapType& mp,const std::string& key,const T& def=T()){
    auto iter=mp.find(key);
    if(iter==mp.end()){
        return def;
    }

    try{
        return boost::lexical_cast<T>(iter->second);
    }catch(...){
        return def;
    }
}

// HTTP 请求文本类
class HttpRequest{
public:
    typedef std::shared_ptr<HttpRequest> ptr;
    typedef std::map<std::string,std::string,CaseInsensitiveLess> MapType;

    HttpRequest(uint8_t version=0x11,bool iskeepalive=true);

    // 获取方法
    Http_Method getMethod() const { return m_method; }
    // 获取版本
    uint8_t getVersion() const { return m_version; }
    // 是否为长连接
    bool isKeepAlive() const { return m_keepalive; }
    // 获取路径
    const std::string& getPath() const { return m_path; }
    // 获取查询参数
    const std::string& getQuery() const { return m_query; }
    // 获取标签
    const std::string& getFragment() const { return m_fragment; }
    // 获取消息体
    const std::string& getBody() const { return m_body; }
    // 获取请求头 map
    const MapType& getHeaders() const { return m_headers; }
    // 获取请求参数 map
    const MapType& getParams() const { return m_params; }
    // 获取Cookies map
    const MapType& getCookies() const { return m_cookies; }

    // 设置方法
    void setMethod(Http_Method meth){ m_method=meth; }
    // 设置版本
    void setVersion(uint8_t ver){ m_version=ver; }
    // 设置路径
    void setPath(const std::string& path){ m_path=path; }
    // 设置查询参数
    void setQuery(const std::string& query){ m_query=query; }
    // 设置标签
    void setFragment(const std::string& frag){ m_fragment=frag; }
    // 设置消息体
    void setBody(const std::string& body){ m_body=body; }
    // 设置长连接
    void setKeepAlive(bool flag){ m_keepalive=flag; }
    // 设置请求头 map
    void setHeaders(const MapType& val){ m_headers=val; }
    // 设置请求参数 map
    void setParams(const MapType& val){ m_headers=val; }
    // 设置Cookies map
    void setCookies(const MapType& val){ m_cookies=val; }

    // 获取指定请求头
    std::string getHeaders(const std::string& key,const std::string& def="") const ;
    // 获取指定请求参数
    std::string getParams(const std::string& key,const std::string& def="") const ;
    // 获取指定Cookies
    std::string getCookies(const std::string& key,const std::string& def="") const ;

    // 设置指定请求头
    void setHeaders(const std::string& key,const std::string& val);
    // 设置指定请求参数
    void setParams(const std::string& key,const std::string& val);
    // 设置指定Cookies
    void setCookies(const std::string& key,const std::string& val);

    // 删除指定请求头
    void delHeaders(const std::string& key);
    // 删除指定请求参数
    void delParams(const std::string& key);
    // 删除指定Cookies
    void delCookies(const std::string& key);

    // 判断是否有指定请求头，有则返回true，并将值保存在指定类型val中
    template<typename T>
    bool checkHeadersAs(const std::string& key,T& val,const T& def=T()){
        return checkAs(m_headers,key,val,def);
    }
    // 判断是否有指定请求参数，有则返回true，并将值保存在指定类型val中
    template<typename T>
    bool checkParamsAs(const std::string& key,T& val,const T& def=T()){
        return checkAs(m_params,key,val,def);
    }
    // 判断是否有指定Cookies，有则返回true，并将值保存在指定类型val中
    template<typename T>
    bool checkCookiesAs(const std::string& key,T& val,const T& def=T()){
        return checkAs(m_cookies,key,val,def);
    }

    // 获取指定请求头，返回特定类型的值
    template<typename T>
    T getHeadersAs(const std::string& key,const T& def=T()){
        return getAs(m_headers,key,def);
    }
    // 获取指定请求参数，返回特定类型的值
    template<typename T>
    T getParamsAs(const std::string& key,const T& def=T()){
        return getAs(m_params,key,def);
    }
    // 获取指定Cookies，返回特定类型的值
    template<typename T>
    T getCookiesAs(const std::string& key,const T& def=T()){
        return getAs(m_cookies,key,def);
    }

    // 转存
    std::ostream& dump(std::ostream& os) const ;
    // 转为字符串
    std::string toString() const ;

private:
    Http_Method m_method;
    uint8_t m_version;          // 0xab: HTTP/a.b
    // 是否为长连接
    bool m_keepalive;

    // 请求路径
    std::string m_path;
    // 请求参数
    std::string m_query;
    // 标签
    std::string m_fragment;
    // 请求体
    std::string m_body;
    // 请求头部 map
    MapType m_headers;
    // 请求参数 map
    MapType m_params;
    // 请求Cookie map
    MapType m_cookies;

};

// Http 响应文本类
class HttpResponse{
public:
    typedef std::shared_ptr<HttpResponse> ptr;
    typedef std::map<std::string,std::string,CaseInsensitiveLess> MapType;

    HttpResponse(uint8_t version=0x11,bool isKeepAlive=true);

    // 获取方法
    Http_Status getStatus() const { return m_status; }
    // 获取版本
    uint8_t getVersion() const { return m_version; }
    // 是否为长连接
    bool isKeepAlive() const { return m_keepalive; }
    // 获取消息体
    const std::string& getBody() const { return m_body; }
    // 获取响应原因
    const std::string& getreact() const { return m_react; }
    // 获取响应头 map
    const MapType& getHeaders() const { return m_headers; }

    // 设置状态
    void setStatus(Http_Status stat){ m_status=stat; }
    // 设置版本
    void setVersion(uint8_t val){ m_version=val; }
    // 设置长连接
    void setKeepAlive(bool flag){ m_keepalive=flag; }
    // 设置消息体
    void setBody(const std::string& body){ m_body=body; }
    // 设置响应原因
    void setreact(const std::string& res){ m_react=res; }
    // 设置响应头 map
    void setHeaders(const MapType& val){ m_headers=val; }

    // 获取指定响应头
    std::string getHeaders(const std::string& key,const std::string& def="") const ;
    // 设置指定响应头
    void setHeaders(const std::string& key,const std::string& val);
    // 删除指定响应头
    void delHeaders(const std::string& key);

    // 判断是否有指定响应头，有则返回true，并将值保存在指定类型val中
    template<typename T>
    bool checkHeadersAs(const std::string& key,T& val,const T& def=T()){
        return checkAs(m_headers,key,val,def);
    }
    // 获取指定响应头，返回特定类型的值
    template<typename T>
    T getHeadersAs(const std::string& key,const T& def=T()){
        return getAs(m_headers,key,def);
    }

    // 转存
    std::ostream& dump(std::ostream& os) const ;
    // 转为字符串
    std::string toString() const ;

private:
    Http_Status m_status;
    uint8_t m_version;
    // 是否为长连接
    bool m_keepalive;

    // 响应消息体
    std::string m_body;
    // 响应码文本
    std::string m_react;
    // 响应头部 map
    MapType m_headers;
    // 响应Cookies数组
    std::vector<std::string> m_cookies;
};



}

}
#endif
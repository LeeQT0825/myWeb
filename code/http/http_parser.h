#ifndef __MYWEB_HTTP_PARSER_H__
#define __MYWEB_HTTP_PARSER_H__

#include "http11_parser.h"
#include "httpclient_parser.h"
#include "http.h"
#include <memory>

namespace myWeb{

namespace http{

// HTTP 请求解析类
class HttpRequestParser{
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    
    HttpRequestParser();

    /*  解析协议
        data: 协议文本
        len: 协议文本内存长度 */
    size_t execute(char* data,size_t len);
    /*  是否完成解析 */
    int isFinished();
    /*  是否有错误 */
    int isError();

    // 获取消息体长度
    size_t getContentLength();
    // 获取 http_parser 结构体
    const http_parser& getParser() const { return m_parser; }
    // 获取请求类对象
    HttpRequest::ptr getReqData() const { return m_request; }
    // 设置错误号
    void setError(int val){ m_error=val; }

private:
    http_parser m_parser;
    HttpRequest::ptr m_request;
    int m_error;

};

// HTTP 响应解析类
class HttpResposeParser{
public:
    typedef std::shared_ptr<HttpResposeParser> ptr;

    HttpResposeParser();

    /*  解析协议
        data: 协议文本
        len: 协议文本内存长度
        chunk: 是否正在解析 chunk */
    size_t execute(char* data,size_t len,bool chunk);
    /*  是否完成解析 */
    int isFinished();
    /*  是否有错误 */
    int isError();

    // 获取消息体长度
    size_t getContentLength();
    // 获取 httpclient_parser 结构体
    const httpclient_parser& getParser() const { return m_parser; }
    // 获取响应类对象
    HttpResponse::ptr getRspData() const { return m_response; }
    // 设置错误号
    void setError(int val){ m_error=val; }

private:
    httpclient_parser m_parser;
    HttpResponse::ptr m_response;
    int m_error;

};



}

}
#endif
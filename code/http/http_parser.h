#ifndef __MYWEB_HTTP_PARSER_H__
#define __MYWEB_HTTP_PARSER_H__

#include "httpserver_parser.h"
#include "httpclient_parser.h"
#include "http.h"
#include "../bytearray.h"
#include <memory>

namespace myWeb{

namespace http{

/* 该解析仅仅解析头部，消息体并不读取。传入字符串经解析后，剩下的就是消息体 */

// HTTP 请求解析类
class HttpRequestParser{
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    
    HttpRequestParser();

    /*  解析协议
        data: 协议文本
        len: 协议文本内存长度
        返回值：-1：错误，>0：解析长度，0：无数据可解析
        注意：解析完后，会将未解析的部分移到缓冲区头部 */
    int execute(char* data,size_t len);
    /*  是否完成解析
            出错：-1，完成：1，未完成：0 */
    int isFinished();
    /*  是否有错误 */
    bool isError();
    // 返回错误号
    int getError() const { return m_error; }

    // 获取消息体长度
    size_t getContentLength();
    // 获取 http_parser 结构体
    const http_parser& getParser() const { return m_parser; }
    // 获取请求类对象
    HttpRequest::ptr getReqData() const { return m_request; }
    // 设置错误号
    void setError(int val){ m_error=val; }

public:
    // 获取存储http请求的缓存大小
    static uint64_t GetReq_BufferSize();
    // 获取存储http消息体的最大大小
    static uint64_t GetReq_maxBodySize();

private:
    http_parser m_parser;
    HttpRequest::ptr m_request;
    /* 错误码：
        1000：invalid method
        1001：invalid version
        1002：invalid field */
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
        chunk: 是否正在解析 chunk
        返回值：-1：错误，>0：解析长度，0：无数据可解析 */
    int execute(char* data,size_t len,bool chunk);
    /*  是否完成解析 */
    int isFinished();
    /*  是否有错误 */
    bool isError();
    // 返回错误号
    int getError() const { return m_error; }

    // 获取消息体长度
    size_t getContentLength();
    // 获取 httpclient_parser 结构体
    const httpclient_parser& getParser() const { return m_parser; }
    // 获取响应类对象
    HttpResponse::ptr getRspData() const { return m_response; }
    // 设置错误号
    void setError(int val){ m_error=val; }

public:
    // 获取存储http响应的缓存大小
    static uint64_t GetRsp_BufferSize();
    // 获取存储http响应消息体的最大大小
    static uint64_t GetRsp_maxBodySize();

private:
    httpclient_parser m_parser;
    HttpResponse::ptr m_response;
    /* 错误码：
        1000：invalid method
        1001：invalid version
        1002：invalid field */
    int m_error;

};



}

}
#endif
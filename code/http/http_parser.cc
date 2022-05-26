#include "http_parser.h"
#include "../log.h"
#include "../config.h"

namespace myWeb{
namespace http{

// 配置部分
static ConfigVar<uint64_t>::ptr config_http_req_BufferSize=Config::Lookup("http.request.buffsize",
                                                                (uint64_t)(4*1024),"http_req_BufferSize");
static ConfigVar<uint64_t>::ptr config_http_req_maxBodySize=Config::Lookup("http.request.maxbodysize",
                                                                (uint64_t)(64*1024*1024),"http_req_maxBodySize");
static ConfigVar<uint64_t>::ptr config_http_rsp_BufferSize=Config::Lookup("http.response.buffsize",
                                                                (uint64_t)(4*1024),"http_rsp_BufferSize");
static ConfigVar<uint64_t>::ptr config_http_rsp_maxBodySize=Config::Lookup("http.response.maxbodysize",
                                                                (uint64_t)(64*1024*1024),"http_rsp_maxBodySize");                                                                                                                                                                                                

static uint64_t s_http_req_BufferSize=0;
static uint64_t s_http_req_maxBodySize=0;
static uint64_t s_http_rsp_BufferSize=0;
static uint64_t s_http_rsp_maxBodySize=0;

struct Http_ConfigInit
{
    Http_ConfigInit(){
        s_http_req_BufferSize=config_http_req_BufferSize->getVal();
        s_http_req_maxBodySize=config_http_req_maxBodySize->getVal();
        s_http_rsp_BufferSize=config_http_rsp_BufferSize->getVal();
        s_http_rsp_maxBodySize=config_http_rsp_maxBodySize->getVal();

        // config 回调
        config_http_req_BufferSize->addListener([](const uint64_t& oldval,const uint64_t& newval){
            s_http_req_BufferSize=newval;
            // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"http_req_BufferSize Changed: "<<newval;
        });
        config_http_req_maxBodySize->addListener([](const uint64_t& oldval,const uint64_t& newval){
            s_http_req_maxBodySize=newval;
            // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"http_req_maxBodySize Changed: "<<newval;
        });
        config_http_rsp_BufferSize->addListener([](const uint64_t& oldval,const uint64_t& newval){
            s_http_rsp_BufferSize=newval;
            // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"http_rsp_BufferSize Changed: "<<newval;
        });
        config_http_rsp_maxBodySize->addListener([](const uint64_t& oldval,const uint64_t& newval){
            s_http_rsp_maxBodySize=newval;
            // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"http_rsp_maxBodySize Changed: "<<newval;
        });
    }
};
static Http_ConfigInit _httpconfig;

uint64_t HttpRequestParser::GetReq_BufferSize(){
    return s_http_req_BufferSize;
}
uint64_t HttpRequestParser::GetReq_maxBodySize(){
    return s_http_req_maxBodySize;
}
uint64_t GetRsp_BufferSize(){
    return s_http_rsp_BufferSize;
}
uint64_t GetRsp_maxBodySize(){
    return s_http_rsp_maxBodySize;
}

// 回调处理

// 解析请求行方法
void on_request_method(void *data, const char *at, size_t length){
    HttpRequestParser* parser=static_cast<HttpRequestParser*>(data);
    Http_Method method=Chars2HttpMethod(at);
    if(method==Http_Method::INVALID_METHOD){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Invalid HTTP request method: "<<std::string(at,length);
        parser->setError(1000);
        return;
    }
    parser->getReqData()->setMethod(method);
}
// 解析请求行URI
void on_request_uri(void *data, const char *at, size_t length){
    // HttpRequestParser* parser=static_cast<HttpRequestParser*>(data);
}
// 解析请求行标签
void on_request_fragment(void *data, const char *at, size_t length){
    HttpRequestParser* parser=static_cast<HttpRequestParser*>(data);
    parser->getReqData()->setFragment(std::string(at,length));
}
// 解析请求行路径
void on_request_path(void *data, const char *at, size_t length){
    HttpRequestParser* parser=static_cast<HttpRequestParser*>(data);
    parser->getReqData()->setPath(std::string(at,length));
}
// 解析请求行请求参数
void on_request_query(void *data, const char *at, size_t length){
    HttpRequestParser* parser=static_cast<HttpRequestParser*>(data);
    parser->getReqData()->setQuery(std::string(at,length));
}
// 解析请求行版本信息
void on_request_version(void *data, const char *at, size_t length){
    HttpRequestParser* parser=static_cast<HttpRequestParser*>(data);
    uint8_t vers;
    if(strncmp(at,"HTTP/1.1",length)==0){
        vers=0x11;
    }else if(strncmp(at,"HTTP/1.0",length)==0){
        vers=0x10;
    }else{
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Invalid HTTP request version: "<<std::string(at,length);
        parser->setError(1001);
        return;
    }
    parser->getReqData()->setVersion(vers);
}
// 解析请求
void on_request_header_done(void *data, const char *at, size_t length){
    // HttpRequestParser* parser=static_cast<HttpRequestParser*>(data);
}
// 解析请求报头
void on_request_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen){
    HttpRequestParser* parser=static_cast<HttpRequestParser*>(data);
    if(vlen<=0){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Invalid HTTP request field value length";
        parser->setError(1002);
        return;
    }
    parser->getReqData()->setHeaders(std::string(field,flen),std::string(value,vlen));
    
}

// 解析状态码
void on_response_reason_phrase(void *data, const char *at, size_t length){
    HttpResposeParser* parser=static_cast<HttpResposeParser*>(data);
    parser->getRspData()->setreact(std::string(at,length));
}
//
void on_response_status_code(void *data, const char *at, size_t length){
    HttpResposeParser* parser=static_cast<HttpResposeParser*>(data);
    Http_Status stat=(Http_Status)atoi(at);
    parser->getRspData()->setStatus(stat);
}
//
void on_response_chunk_size(void *data, const char *at, size_t length){
    // HttpResposeParser* parser=static_cast<HttpResposeParser*>(data);

}
//
void on_response_version(void *data, const char *at, size_t length){
    HttpResposeParser* parser=static_cast<HttpResposeParser*>(data);
    uint8_t vers;
    if(strncmp(at,"HTTP/1.1",length)==0){
        vers=0x11;
    }else if(strncmp(at,"HTTP/1.0",length)==0){
        vers=0x10;
    }else{
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Invalid HTTP response version: "<<std::string(at,length);
        parser->setError(1001);
        return;
    }
    parser->getRspData()->setVersion(vers);
}
//
void on_response_header_done(void *data, const char *at, size_t length){
    // HttpResposeParser* parser=static_cast<HttpResposeParser*>(data);

}
//
void on_response_last_chunk(void *data, const char *at, size_t length){
    // HttpResposeParser* parser=static_cast<HttpResposeParser*>(data);

}
//
void on_response_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen){
    HttpResposeParser* parser=static_cast<HttpResposeParser*>(data);
    if(vlen<=0){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Invalid HTTP response field value length";
        parser->setError(1002);
        return;
    }
    parser->getRspData()->setHeaders(std::string(field,flen),std::string(value,vlen));
}

// HTTP 请求解析类

HttpRequestParser::HttpRequestParser()
                        :m_error(0){
    m_request.reset(new HttpRequest());
    http_parser_init(&m_parser);
    m_parser.http_field=on_request_http_field;
    m_parser.request_method=on_request_method;
    m_parser.request_uri=on_request_uri;
    m_parser.fragment=on_request_fragment;
    m_parser.request_path=on_request_path;
    m_parser.query_string=on_request_query;
    m_parser.http_version=on_request_version;
    m_parser.header_done=on_request_header_done;
    m_parser.data=this;
}

int HttpRequestParser::execute(char* data,size_t len){
    size_t offset=http_parser_execute(&m_parser,data,len,0);
    // 将未读完的移到data起始处
    memmove(data,data+offset,(len-offset));
    return offset;
}
int HttpRequestParser::isFinished(){
    return http_parser_finish(&m_parser);
}
bool HttpRequestParser::isError(){
    return m_error || http_parser_has_error(&m_parser);
}
size_t HttpRequestParser::getContentLength(){
    return m_request->getHeadersAs<size_t>("content-length",0);
}


// HTTP 响应解析类
HttpResposeParser::HttpResposeParser()
                            :m_error(0){
    m_response.reset(new HttpResponse());
    httpclient_parser_init(&m_parser);
    m_parser.http_field=on_response_http_field;
    m_parser.reason_phrase=on_response_reason_phrase;
    m_parser.status_code=on_response_status_code;
    m_parser.chunk_size=on_response_chunk_size;
    m_parser.http_version=on_response_version;
    m_parser.header_done=on_response_header_done;
    m_parser.last_chunk=on_response_last_chunk;
    m_parser.data=this;
}

int HttpResposeParser::execute(char* data,size_t len,bool chunk){
    if(chunk){
        httpclient_parser_init(&m_parser);
    }
    size_t offset=httpclient_parser_execute(&m_parser,data,len,0);
    // 将未读完的移到data起始处
    memmove(data,data+offset,(len-offset));
    return offset;
}
int HttpResposeParser::isFinished(){
    return httpclient_parser_finish(&m_parser);
}
bool HttpResposeParser::isError(){
    return m_error || httpclient_parser_has_error(&m_parser);
}
size_t HttpResposeParser::getContentLength(){
    return m_response->getHeadersAs<size_t>("content-length",0);
}

}
}
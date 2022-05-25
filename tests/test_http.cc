#include "../code/http/http.h"
#include "../code/http/http_parser.h"
#include "../code/log.h"
#include "../code/config.h"
#include "../code/macro.h"

const char request[]=   "GET /favicon.ico HTTP/1.1\r\n"
                        "Host: localhost:12345\r\n"
                        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:96.0) Gecko/20100101 Firefox/96.0\r\n"
                        "Accept: image/avif,image/webp,*/*\r\n"
                        "Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "Connection: keep-alive\r\n"
                        "Referer: http://localhost:12345/\r\n"
                        "Sec-Fetch-Dest: image\r\n"
                        "Sec-Fetch-Mode: no-cors\r\n"
                        "Sec-Fetch-Site: same-origin\r\n"
                        "Cache-Control: max-age=0\r\n\r\n"
                        "aaabbbcccddd";
;

const char response[]=  "HTTP/1.1 200 OK\r\n"
                        "Accept-Ranges: bytes\r\n"
                        "Cache-Control: no-cache\r\n"
                        "Connection: keep-alive\r\n"
                        "Content-Type: text/html\r\n"
                        "Date: Mon, 23 May 2022 12:00:26 GMT\r\n"
                        "P3p: CP=\" OTI DSP COR IVA OUR IND COM \"\r\n"
                        "P3p: CP=\" OTI DSP COR IVA OUR IND COM \"\r\n"
                        "Pragma: no-cache\r\n"
                        "Server: BWS/1.1\r\n"
                        "Set-Cookie: BAIDUID=120C69256AF8D3A577DCD4B1DDB6CFFF:FG=1; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com\r\n"
                        "Set-Cookie: BIDUPSID=120C69256AF8D3A577DCD4B1DDB6CFFF; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com\r\n"
                        "Set-Cookie: PSTM=1653307226; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com\r\n"
                        "Set-Cookie: BAIDUID=120C69256AF8D3A55DCB1F3F7FFD2B7F:FG=1; max-age=31536000; expires=Tue, 23-May-23 12:00:26 GMT; domain=.baidu.com; path=/; version=1; comment=bd\r\n"
                        "Traceid: 165330722627604213868557367006788672020\r\n"
                        "Vary: Accept-Encoding\r\n"
                        "X-Frame-Options: sameorigin\r\n"
                        "X-Ua-Compatible: IE=Edge,chrome=1\r\n\r\n"
                        "<html>\r\n"
                        "aaabbbcccdddeeefffggg\r\n"
                        "</html>\r\n";

void test_http(){
    myWeb::http::HttpRequest::ptr req(new myWeb::http::HttpRequest(0x11,true));
    req->setPath("/aaabbbccc");
    req->setQuery("ie=utf-8&f=8");
 
    req->setHeaders("Host","www.baidu.com");
    req->setBody("Hello World!\n");

    req->dump(std::cout)<<std::endl;

    myWeb::http::HttpResponse::ptr resp(new myWeb::http::HttpResponse(0x11,true));
    resp->setStatus(myWeb::http::Http_Status::NOT_FOUND);
    resp->setBody("bye");
    resp->dump(std::cout)<<std::endl;
    
}

void test_request_parser(){
    std::string tmp=request;
    int len=tmp.size();
    myWeb::http::HttpRequestParser::ptr http_req_parser(new myWeb::http::HttpRequestParser());
    int ret=0;
    ret=http_req_parser->execute(&tmp[0],len);

    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"http_request_parser info:\n execute ret= "<<ret<<"/"<<tmp.size()
                                        <<"\n error= "<<http_req_parser->getError()
                                        <<"\n is finish= "<<http_req_parser->isFinished()
                                        <<"\n Content Length= "<<http_req_parser->getContentLength()<<"\n";
    http_req_parser->getReqData()->dump(std::cout);

    tmp.resize(len-ret);
    std::cout<<"http_body: \n"<<tmp<<std::endl;
}

void test_response_parser(){
    std::string tmp=response;
    int len=tmp.size();
    myWeb::http::HttpResposeParser::ptr http_rsp_parser(new myWeb::http::HttpResposeParser());
    int ret=0;
    ret=http_rsp_parser->execute(&tmp[0],len,false);

    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"\n execute ret= "<<ret<<"/"<<len
                                        <<"\n error= "<<http_rsp_parser->getError()
                                        <<"\n is finish= "<<http_rsp_parser->isFinished()
                                        <<"\n Content Length= "<<http_rsp_parser->getContentLength()<<"\n";
    std::cout<<http_rsp_parser->getRspData()->toString()<<std::endl;
    // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<tmp.size();
    tmp.resize(len-ret);
    std::cout<<tmp<<std::endl;
}

int main(int argc,char** argv){
    LOADYAML;
    test_request_parser();
    // test_response_parser();

    return 0;
}
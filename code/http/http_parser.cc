#include "http_parser.h"
#include "../log.h"

namespace myWeb{
namespace http{

// HTTP 请求解析类

HttpRequestParser::HttpRequestParser(){
    
}

size_t HttpRequestParser::execute(char* data,size_t len){

}
int HttpRequestParser::isFinished(){

}
int HttpRequestParser::isError(){

}
size_t HttpRequestParser::getContentLength(){

}


// HTTP 响应解析类
HttpResposeParser::HttpResposeParser(){
    
}

size_t HttpResposeParser::execute(char* data,size_t len,bool chunk){

}
int HttpResposeParser::isFinished(){

}
int HttpResposeParser::isError(){

}
size_t HttpResposeParser::getContentLength(){

}

}
}
#include "../code/http/http.h"

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

int main(int argc,char** argv){
    test_http();

    return 0;
}
#include "../code/myweb.h"

void run(){
    std::vector<myWeb::Address::ptr> addrs;
    myWeb::Address::Lookup(addrs,"192.168.8.106:12345");
    if(addrs.empty())   return;
    myWeb::Address::ptr addr=addrs[0];

    myWeb::http::Http_Server::ptr server(new myWeb::http::Http_Server(false));
    while(!server->bind_listen(addr)){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"bind "<<addr->toString()<<" failed";
        sleep(1);    
    }
    auto sd=server->getServletDispatch();
    sd->addServlet("/lqt/xx",[](myWeb::http::HttpRequest::ptr req,
                                myWeb::http::HttpResponse::ptr rsp,
                                myWeb::http::HttpSession::ptr session){
        rsp->setBody(req->toString());
        return 0;
    });
    sd->addObsServlet("/lqt/*",[](myWeb::http::HttpRequest::ptr req,
                                myWeb::http::HttpResponse::ptr rsp,
                                myWeb::http::HttpSession::ptr session){
        rsp->setBody("Glob:\r\n"+req->toString());
        return 0;
    });

    server->start();
}

int main(int args,char** argv){
    LOADYAML;
    myWeb::IOManager::ptr iom(new myWeb::IOManager(8));
    iom->schedule(run);

    return 0;
}
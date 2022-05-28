#include "../code/myweb.h"

class Echo_Server:public myWeb::TCP_Server{
public:
    Echo_Server(){}

    void handleClient(myWeb::mySocket::ptr client_sock);

};

void Echo_Server::handleClient(myWeb::mySocket::ptr client_sock){
    // 功能：接收信息，通过序列化
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"client_sock info: "<<client_sock->toString();

    myWeb::http::HttpSession::ptr session(new myWeb::http::HttpSession(client_sock,true));
    myWeb::http::HttpRequest::ptr http_req=session->recvRequest();
    if(http_req){
        // 打印 http_request
        std::string recv_buff=http_req->toString();
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"recv_len: "<<recv_buff.size()<<"\n"<<recv_buff;
    }
}

void run(){
    std::vector<myWeb::Address::ptr> addrs;
    myWeb::Address::Lookup(addrs,"0.0.0.0:12345");
    if(addrs.empty())   return;
    myWeb::Address::ptr addr=addrs[0];

    Echo_Server::ptr echo_server(new Echo_Server);
    if(echo_server->bind_listen(addr)){
        sleep(2);
        echo_server->start();
    }
}

void test_http_server(){
    std::vector<myWeb::Address::ptr> addrs;
    myWeb::Address::Lookup(addrs,"0.0.0.0:12345");
    if(addrs.empty())   return;
    myWeb::Address::ptr addr=addrs[0];

    myWeb::http::Http_Server::ptr http_server(new myWeb::http::Http_Server(true));
    if(http_server->bind_listen(addr)){
        sleep(2);
        http_server->start();
    }
}


int main(int argc,char** argv){
    LOADYAML;
    myWeb::IOManager::ptr iom(new myWeb::IOManager(3));
    iom->schedule(test_http_server);

    return 0;
}
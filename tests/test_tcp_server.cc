#include "../code/tcp_server.h"
#include "../code/iomanager.h"
#include "../code/macro.h"

void run(){
    std::multimap<std::string,std::pair<myWeb::Address::ptr,uint32_t> > results;
    myWeb::Address::getInterfaceAddress(results);

    auto iter=results.find("wlp3s0");
    if(iter!=results.end()){
        myWeb::IPv4_Address::ptr addr=std::dynamic_pointer_cast<myWeb::IPv4_Address>((*iter).second.first);
        addr->setPort(12345);
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<addr->toString();

        myWeb::TCP_Server::ptr tcp_server(new myWeb::TCP_Server());
        tcp_server->bind_listen(addr);

        tcp_server->start();
        myWeb::IOManager::getThis()->addTimer(5*1000,[tcp_server](){
           tcp_server->stop(); 
        });

        // sleep(5);
    }
}

void run2(){
    std::vector<myWeb::Address::ptr> addrs;
    myWeb::Address::Lookup(addrs,"0.0.0.0:12345");
    if(addrs.empty())   return;
    myWeb::Address::ptr addr=addrs[0];

    myWeb::TCP_Server::ptr tcp_server(new myWeb::TCP_Server());

    std::vector<myWeb::Address::ptr> f_addrs;
    if(tcp_server->bind_listen(addr)){
        sleep(2);
        tcp_server->start();
    }

    // 定时关闭
    // myWeb::IOManager::getThis()->addTimer(5*1000,[tcp_server](){
    //     tcp_server->stop(); 
    // });
}

int main(int argc,char** argv){
    
    myWeb::IOManager::ptr iom(new myWeb::IOManager(5));
    iom->schedule(run2);

    return 0;
}
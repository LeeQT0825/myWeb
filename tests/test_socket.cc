#include <vector>
#include <map>
#include "../code/myweb.h"
#include "../code/address.h"
#include "../code/socket.h"

void test_server(){
    std::multimap<std::string,std::pair<myWeb::Address::ptr,uint32_t> > results;
    myWeb::Address::getInterfaceAddress(results);
    // myWeb::IOManager* iomanager=myWeb::IOManager::getThis();

    auto iter=results.find("wlp3s0");
    if(iter!=results.end()){
        myWeb::IPv4_Address::ptr addr=std::dynamic_pointer_cast<myWeb::IPv4_Address>((*iter).second.first);
        addr->setPort(12345);
        
        myWeb::mySocket::ptr listen_sock=myWeb::mySocket::CreateTCPsocket4();
        listen_sock->bind(addr);
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<listen_sock->toString();

        if(listen_sock->listen()){
            myWeb::mySocket::ptr conn_sock=listen_sock->accept();
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<conn_sock->toString();

            std::string recv_buf;
            recv_buf.resize(4096);
            int ret=conn_sock->recv(&recv_buf[0],sizeof(recv_buf),0);
            
            if(ret<0){
                INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"recv ret= "<<ret<<" errno= "<<errno;
                return;
            }

            recv_buf.resize(ret);
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<recv_buf;
            sleep(5);
        }
        
    } 
}

void test_Lookup(){
    std::vector<myWeb::Address::ptr> addrs;
    bool ret=myWeb::Address::Lookup(addrs,"LocalHost:http",AF_INET,SOCK_STREAM);
    if(!ret){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Lookup error";
        return;
    }

    for(size_t i=0;i<addrs.size();++i){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<i<<" -- "<<addrs[i]->toString();
    }
}

void test_interface(){
    std::multimap<std::string,std::pair<myWeb::Address::ptr,uint32_t> > results;
    bool ret=myWeb::Address::getInterfaceAddress(results);
    if(!ret){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"getInterfaceAddress error";
        return;
    }

    for(auto& i: results){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<i.first<<" --- "<<i.second.first->toString()<<" -- "<<i.second.second;
    }
}

void test_ipv4(){
    myWeb::IP_Address::ptr addr=myWeb::IP_Address::Create_addr("180.101.49.11",80);
    if(addr){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<addr->toString();
    }
}

int main(int argc,char** argv){
    LOADYAML;

    // if(argc<=2){
    //     std::cout<<"usage: "<<basename(argv[0])<<" IP_addr Port_num"<<std::endl;
    //     return 1;
    // }

    // const char* ip=argv[1];
    // uint16_t port=atoi(argv[2]);

    myWeb::IOManager::ptr iomanager(new myWeb::IOManager(5));

    // test_Lookup();

    // test_ipv4();

    // test_interface();

    iomanager->schedule(std::bind(test_server));

    return 0;
}
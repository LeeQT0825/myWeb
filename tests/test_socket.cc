#include <vector>
#include <map>
#include <string>
#include "../code/myweb.h"
#include "../code/address.h"
#include "../code/socket.h"

void test_server(){
    std::multimap<std::string,std::pair<myWeb::Address::ptr,uint32_t> > results;
    myWeb::Address::getInterfaceAddress(results);

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

void test_client(const char* ip){
    std::multimap<std::string,std::pair<myWeb::Address::ptr,uint32_t> > results;
    myWeb::Address::getInterfaceAddress(results);

    auto iter=results.find("wlp3s0");
    if(iter!=results.end()){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"here";
        std::vector<myWeb::Address::ptr> vec;
        myWeb::Address::ptr dst_addr;

        myWeb::Address::Lookup(vec,ip);
        if(!vec.empty()){
            dst_addr=vec[0];
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"get dst_addr: "<<dst_addr->toString();
        }

        myWeb::mySocket::ptr sock=myWeb::mySocket::CreateTCPsocket4();
        if(sock->connect(dst_addr)){
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<sock->toString();

            // const char send_buff[]="GET / HTTP/1.1\r\n\r\n";        // 注意写法
            std::string send_buff="GET / HTTP/1.1\r\n\r\n"; 
            int ret=sock->send(&send_buff[0],sizeof(send_buff));
            MYWEB_ASSERT_2(ret!=-1,"send failed");
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"send success";

            std::string recv_buff;
            recv_buff.resize(4096);
            ret=sock->recv(&recv_buff[0],recv_buff.size());
            // char recv_buff[4096];
            // ret=sock->recv(recv_buff,sizeof(recv_buff),MSG_WAITALL);
            MYWEB_ASSERT_2(ret!=-1,"recv failed");
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"recv success";

            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"\n"<<recv_buff;
            // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<recv_io;
        }
        sock->close();
             
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

    const char* ip=argv[1];
    // uint16_t port=atoi(argv[2]);

    myWeb::IOManager::ptr iomanager(new myWeb::IOManager(5));

    // test_Lookup();

    // test_ipv4();

    // test_interface();

    // iomanager->schedule(std::bind(test_server));

    iomanager->schedule(std::bind(test_client,ip));

    return 0;
}
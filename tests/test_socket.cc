#include "../code/myweb.h"
#include "../code/address.h"

void test_address(const char* ip,uint16_t port){
    
}

void test_Lookup(){
    std::vector<myWeb::Address::ptr> addrs;
    bool ret=myWeb::Address::Lookup(addrs,"www.baidu.com:http",AF_INET,SOCK_STREAM);
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

    // test_address(ip,port);

    test_ipv4();

    return 0;
}
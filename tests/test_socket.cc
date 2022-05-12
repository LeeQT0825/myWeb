#include "../code/myweb.h"
#include "../code/address.h"

void test_address(const char* ip,uint16_t port){
    
}

void test(){
    std::vector<myWeb::Address::ptr> addrs;
    bool ret=myWeb::Address::Lookup(addrs,"www.baidu.com");
    if(!ret){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Lookup error";
        return;
    }

    for(size_t i=0;i<addrs.size();++i){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<i<<" -- "<<addrs[i]->toString();
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

    test();

    return 0;
}
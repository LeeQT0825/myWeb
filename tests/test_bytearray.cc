#include "../code/myweb.h"
#include "../code/bytearray.h"
#include <time.h>
#include <typeinfo>

void test_val(){
#define XX(type,len,write_func,read_func,node_size) {\
    myWeb::ByteArray::ptr ba(new myWeb::ByteArray(node_size)); \
    std::vector<type> vec; \
    srand((unsigned)time(NULL)); \
    std::string tp=#type; \
    std::string f="float"; \
    std::string d="double"; \
    for(size_t i=0;i<len;++i){  \
        if(tp==f || tp==d){ \
            vec.push_back(rand()/double(RAND_MAX)); \
        }else{  \
            vec.push_back(rand());  \
        } \
    }   \
    for(auto& i:vec){ \
        ba->write_func(i); \
    } \
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"write: data_size="<<ba->getSize(); \
    \
    ba->setPosition(0); \
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<ba->toString(); \
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<ba->toHexString(); \
    for(size_t i=0;i<len;++i){ \
        type v=ba->read_func(); \
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<i+1<<"--"<<vec[i]<<"--"<<v<<"--"<<std::hex<<v; \
    } \
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"\n"#write_func"/"#read_func<<"("#type")\n len="<<len \
                                    <<"\n node_size="<<node_size \
                                    <<"\n node_count="<<ba->getNodeCount() \
                                    <<"\n data_size="<<ba->getSize(); \
    }

    // XX(int8_t,20,writef_int8,readf_int8,1);
    // XX(uint8_t,20,writef_uint8,readf_uint8,1);
    // XX(int16_t,20,writef_int16,readf_int16,1);
    // XX(uint64_t,20,writef_uint64,readf_uint64,1);
    XX(uint64_t,20,writef_uint64,readf_uint64,3);
    // XX(float,20,write_float,read_float,1);
    // XX(uint64_t,20,writef_uint64,readf_uint64,4);
    // XX(int64_t,20,writef_int64,readf_int64,4);
    XX(int64_t,20,writev_int64,readv_int64,1);
    // XX(uint64_t,20,writev_uint64,readv_uint64,1);

#undef XX    
}

void test_file(){
#define XX(type,len,write_func,read_func,node_size) {\
    myWeb::ByteArray::ptr ba(new myWeb::ByteArray(node_size)); \
    std::vector<type> vec; \
    srand((unsigned)time(NULL)); \
    std::string tp=#type; \
    std::string f="float"; \
    std::string d="double"; \
    for(size_t i=0;i<len;++i){  \
        if(tp==f || tp==d){ \
            vec.push_back(rand()/double(RAND_MAX)); \
        }else{  \
            vec.push_back(rand());  \
        } \
    }   \
    for(auto& i:vec){ \
        ba->write_func(i); \
    } \
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"write: data_size="<<ba->getSize(); \
    \
    ba->setPosition(0); \
    MYWEB_ASSERT(ba->readToFile("../tmp/"#type".dat")); \
    \
    myWeb::ByteArray::ptr ba2(new myWeb::ByteArray(node_size)); \
    ba2->writeFromFile("../tmp/"#type".dat"); \
    ba->setPosition(0); \
    ba2->setPosition(0); \
    for(size_t i=0;i<len;++i){ \
        type v=ba->read_func(); \
        type v2=ba2->read_func(); \
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<i+1<<"--"<<vec[i]<<"--"<<v<<"--"<<v2; \
    } \
    \
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"\n"#write_func"/"#read_func<<"("#type")\n len="<<len \
                                    <<"\n node_size="<<node_size \
                                    <<"\n node_count="<<ba->getNodeCount() \
                                    <<"\n data_size="<<ba->getSize(); \
}

    // XX(int8_t,20,writef_int8,readf_int8,1);
    // XX(uint8_t,20,writef_uint8,readf_uint8,1);
    // XX(int16_t,20,writef_int16,readf_int16,1);
    XX(uint64_t,20,writef_uint64,readf_uint64,1);
    // XX(uint64_t,20,writef_uint64,readf_uint64,3);
    XX(float,20,write_float,read_float,1);
    // XX(uint64_t,20,writef_uint64,readf_uint64,4);
    // XX(int64_t,20,writef_int64,readf_int64,4);
    XX(int64_t,20,writev_int64,readv_int64,1);
    XX(uint64_t,20,writev_uint64,readv_uint64,1);

#undef XX    
}

int main(int argc,char** argv){
    LOADYAML;
    test_val();

    return 0;
}
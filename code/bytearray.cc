#include "bytearray.h"
#include "util.h"
#include <string.h>

namespace myWeb{

ByteArray::Node::Node(size_t s)
                    :ptr(new char[s])
                    ,next(nullptr)
                    ,size(s){}
ByteArray::Node::Node()
                    :ptr(nullptr)
                    ,next(nullptr)
                    ,size(0){}
ByteArray::Node::~Node(){
    if(ptr){
        delete[] ptr;
    }
}

ByteArray::ByteArray(size_t node_size)
                :m_NodeSize(node_size)
                ,m_position(0)
                ,m_capacity(node_size)
                ,m_size(0)
                ,m_endian(MYWEB_BIG_ENDIAN)
                ,m_root(new Node(node_size))
                ,m_curr(m_root){}
ByteArray::~ByteArray(){
    Node* tmp=m_root;
    while(tmp){
        m_curr=tmp;
        tmp=tmp->next;
        m_curr->~Node();
    }
}

void ByteArray::writef_int8(int8_t val){
    write(&val,sizeof(val));
}
void ByteArray::writef_uint8(uint8_t val){
    write(&val,sizeof(val));
}
void ByteArray::writef_int16(int16_t val){
    if(m_endian!=MYWEB_ORDER){
        val=ByteSwap(val);
    }
    write(&val,sizeof(val));
}
void ByteArray::writef_uint16(uint16_t val){
    if(m_endian!=MYWEB_ORDER){
        val=ByteSwap(val);
    }
    write(&val,sizeof(val));
}
void ByteArray::writef_int32(int32_t val){
    if(m_endian!=MYWEB_ORDER){
        val=ByteSwap(val);
    }
    write(&val,sizeof(val));
}
void ByteArray::writef_uint32(uint32_t val){
    if(m_endian!=MYWEB_ORDER){
        val=ByteSwap(val);
    }
    write(&val,sizeof(val));
}
void ByteArray::writef_int64(int64_t val){
    if(m_endian!=MYWEB_ORDER){
        val=ByteSwap(val);
    }
    write(&val,sizeof(val));
}
void ByteArray::writef_uint64(uint64_t val){
    if(m_endian!=MYWEB_ORDER){
        val=ByteSwap(val);
    }
    write(&val,sizeof(val));
}

// 负数压缩：Zigzag
// 编码
static uint32_t EncodeZigzag32(const int32_t& val){
    if(val<0){
        return ((uint32_t)(-val))*2-1;
    }else{
        return val*2;
    }
}
static uint64_t EncodeZigzag64(const int64_t& val){
    if(val<0){
        return ((uint64_t)(-val))*2-1;
    }else{
        return val*2;
    }
}
// 解码
static int32_t DecodeZigzag32(const uint32_t& val){
    return (val>>1)^-(val & 1);     // 单目运算符- 优于异或^
}
static int64_t DecodeZigzag64(const uint64_t& val){
    return (val>>1)^-(val & 1);
}

void ByteArray::writev_int32(int32_t val){
    writev_uint32(EncodeZigzag32(val));
}
void ByteArray::writev_uint32(uint32_t val){
    uint8_t tmp[5];                     // protobuf压缩模式，最高保留5个字节
    uint8_t i=0;
    while(val >= 0x80){                 // 每个字节高阶位代表后面还有没有数据，1为有
        tmp[i++]=(val & 0x7f) | 0x80;   // 其他位会被截断
        val >>= 7;
    }
    tmp[i++]=val;   // 注意i++
    write(tmp,i);
}
void ByteArray::writev_int64(int64_t val){
    writev_uint64(EncodeZigzag64(val));
}
void ByteArray::writev_uint64(uint64_t val){
    uint8_t tmp[10];
    uint8_t i=0;
    while(val >= 0x80){
        tmp[i++]=(val & 0x7f) | 0x80;
        val >>= 7;
    }
    tmp[i++]=val;   // 注意i++
    write(tmp,i);
}
void ByteArray::write_float(float val){
    uint32_t tmp;
    memcpy(&tmp,&val,sizeof(val));
    writef_uint32(tmp);
}
void ByteArray::write_double(double val){
    uint64_t tmp;
    memcpy(&tmp,&val,sizeof(val));
    writef_uint64(tmp);
}
void ByteArray::writef_string_16(const std::string& val){
    writef_uint16(val.size());
    write(val.c_str(),val.size());
}
void ByteArray::writef_string_32(const std::string& val){
    writef_uint32(val.size());
    write(val.c_str(),val.size());
}
void ByteArray::writef_string_64(const std::string& val){
    writef_uint64(val.size());
    write(val.c_str(),val.size());
}
void ByteArray::writev_string(const std::string& val){
    writev_uint64(val.size());
    write(val.c_str(),val.size());
}
void ByteArray::write_string_WithoutLength(const std::string& val){
    write(val.c_str(),val.size());
}

int8_t ByteArray::readf_int8(){
    int8_t buf;
    read(&buf,sizeof(int8_t));
    return buf;
}
uint8_t ByteArray::readf_uint8(){
    uint8_t buf;
    read(&buf,sizeof(uint8_t));
    return buf;
}

#define XX(type) \
    type buf;\
    read(&buf,sizeof(type));\
    if(m_endian != MYWEB_ORDER){\
        return ByteSwap(buf);\
    }else{\
        return buf;\
    }

int16_t ByteArray::readf_int16(){
    int16_t buf;
    if(m_endian!=MYWEB_ORDER){
        
    }
}
uint16_t ByteArray::readf_uint16(){
    XX(uint16_t);
}
int32_t ByteArray::readf_int32(){
    XX(int32_t);
}
uint32_t ByteArray::readf_uint32(){
    XX(uint32_t);
}
int64_t ByteArray::readf_int64(){
    XX(int64_t);
}
uint64_t ByteArray::readf_uint64(){
    XX(uint64_t);
}
#undef XX
int32_t ByteArray::readv_int32(){
    return DecodeZigzag32(readv_uint32());
}
uint32_t ByteArray::readv_uint32(){
    uint32_t buf=0;
    for(int i=0;i<32;i+=8){                     // TODO : 这样不会将高阶位数据丢失么？
        // 一个字节一个字节地读出来
        uint8_t one_byte=readf_uint8();
        if(one_byte<0x80){
            buf |= ((uint32_t)one_byte)<<i;     // 将该字节放置到正确字节位上
            break;
        }else{
            buf |= ((uint32_t)(one_byte & 0x7f) | 0x80)<<i;
        }
    }
    return buf;
}
int64_t ByteArray::readv_int64(){
    return DecodeZigzag64(readv_uint64());
}
uint64_t ByteArray::readv_uint64(){
    uint64_t buf=0;
    for(int i=0;i<64;i+=8){                     // TODO : 这样不会将高阶位数据丢失么？
        // 一个字节一个字节地读出来
        uint8_t one_byte=readf_uint8();
        if(one_byte<0x80){
            buf |= ((uint32_t)one_byte)<<i;     // 将该字节放置到正确字节位上
            break;
        }else{
            buf |= ((uint32_t)(one_byte & 0x7f) | 0x80)<<i;
        }
    }
    return buf;
}
float ByteArray::read_float(){
    uint32_t val=readf_uint32();
    float result;
    memcpy(&result,&val,sizeof(uint32_t));
    return result;
}
double ByteArray::read_double(){
    uint64_t val=readf_uint64();
    double result;
    memcpy(&result,&val,sizeof(uint64_t));
    return result;
}
std::string ByteArray::readf_string_16(){
    uint16_t len=readf_uint16();
    std::string buf;
    buf.resize(len);
    read(&buf[0],len);
    return buf;
}
std::string ByteArray::readf_string_32(){
    uint32_t len=readf_uint32();
    std::string buf;
    buf.resize(len);
    read(&buf[0],len);
    return buf;
}
std::string ByteArray::readf_string_64(){
    uint64_t len=readf_uint64();
    std::string buf;
    buf.resize(len);
    read(&buf[0],len);
    return buf;
}
std::string ByteArray::readv_string(){
    uint64_t len=readv_uint64();
    std::string buf;
    buf.resize(len);
    read(&buf[0],len);
    return buf;
}
void ByteArray::clear(){
    m_position=m_size=0;
    m_capacity=m_NodeSize;
    Node* tmp=m_root->next;
    while(tmp){
        m_curr=tmp;
        tmp=tmp->next;
        m_curr->~Node();
    }
    m_curr=m_root;
    m_root->next=nullptr;
}
bool ByteArray::isLittleEndian() const {
    return m_endian==MYWEB_LITTLE_ENDIAN;
}
void ByteArray::setEndian(bool islittle){
    if(islittle){
        m_endian=MYWEB_LITTLE_ENDIAN;
    }else{
        m_endian=MYWEB_BIG_ENDIAN;
    }
}
void ByteArray::write(const void* src_buf,size_t size){
    
}
void ByteArray::read(void* dst_buf,size_t size){

}
void ByteArray::setPosition(size_t pos){

}
bool ByteArray::writeToFile(const std::string& filename){

}
bool ByteArray::readFromFile(const std::string& filename){

}
void ByteArray::addCapacity(size_t size){

}


}
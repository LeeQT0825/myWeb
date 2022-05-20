#include "bytearray.h"
#include "util.h"
#include "log.h"
#include <string.h>
#include <fstream>
#include <math.h>
#include <sstream>

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
                ,m_datsize(0)
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
    XX(int16_t);
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
    uint32_t buf=0;     // result
    for(size_t i=0;i<32;i+=7){
        // 一个字节一个字节地读出来
        uint8_t one_byte=readf_uint8();
        if(one_byte<0x80){      // 无下一字节
            buf |= ((uint32_t)one_byte)<<i;
            break;
        }else{
            buf |= (uint32_t)(one_byte & 0x7f)<<i;
        }
    }
    return buf;
}
int64_t ByteArray::readv_int64(){
    return DecodeZigzag64(readv_uint64());
}
uint64_t ByteArray::readv_uint64(){
    uint64_t buf=0;
    for(size_t i=0;i<64;i+=7){
        uint8_t one_byte=readf_uint8();
        if(one_byte<0x80){
            buf |= ((uint32_t)one_byte)<<i;     // 将该字节放置到正确字节位上
            break;
        }else{
            buf |= (uint32_t)(one_byte & 0x7f)<<i;
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
    m_position=m_datsize=0;
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
    if(size==0){
        return;
    }
    addCapacity(size);

    size_t n_pos=m_position % m_NodeSize;       // 当前节点中的位置
    size_t n_rstcap=m_curr->size-n_pos;         // 当前节点剩余容量
    size_t bpos=0;                              // src_buf 待读写的位置

    while(size>0){
        if(n_rstcap>=size){
            // 当前节点能装下
            memcpy(m_curr->ptr+n_pos,(const char*)src_buf+bpos,size);
            if((n_pos+size) == m_curr->size){
                // 当前节点用尽
                m_curr=m_curr->next;
            }
            m_position+=size;
            bpos+=size;
            size-=size;
        }else{
            memcpy(m_curr->ptr+n_pos,(const char*)src_buf+bpos,n_rstcap);
            m_position+=n_rstcap;
            bpos+=n_rstcap;
            size-=n_rstcap;
            m_curr=m_curr->next;
            n_rstcap=m_curr->size;
            n_pos=0;
        }
    }

    if(m_position>m_datsize){
        m_datsize=m_position;
    }
}
void ByteArray::read(void* dst_buf,size_t size){
    if(size>getRestRdSize()){
        throw std::out_of_range("not enough len");
    }

    size_t n_pos=m_position % m_NodeSize;       // 当前节点中的位置
    size_t n_rstcap=m_curr->size-n_pos;         // 当前节点剩余容量
    size_t bpos=0;                              // src_buf 待读写的位置

    while(size>0){
        if(n_rstcap>=size){
            // 剩下的数据都在当前节点内
            memcpy((char*)dst_buf+bpos,m_curr->ptr+n_pos,size);
            if(n_pos+size == m_curr->size){
                // 当前节点用尽
                m_curr=m_curr->next;
            }
            m_position+=size;
            bpos+=size;
            size-=size;
        }else{
            memcpy((char*)dst_buf+bpos,m_curr->ptr+n_pos,n_rstcap);
            m_position+=n_rstcap;
            bpos+=n_rstcap;
            size-=n_rstcap;
            m_curr=m_curr->next;
            n_rstcap=m_curr->size;
            n_pos=0;
        }
    }
}
void ByteArray::read(void* dst_buf,size_t size,size_t position) const {
    if(size>m_datsize-position){
        throw std::out_of_range("not enough len");
    }
    
    size_t n_pos=position % m_NodeSize;       // 当前节点中的位置
    size_t n_rstcap=m_curr->size-n_pos;         // 当前节点剩余容量
    size_t bpos=0;                              // src_buf 待读写的位置
    Node* cur=m_curr;

    while(size>0){
        if(n_rstcap>=size){
            // 剩下的数据都在当前节点内
            memcpy((char*)dst_buf+bpos,cur->ptr+n_pos,size);
            position+=size;
            bpos+=size;
            size-=size;
        }else{
            memcpy((char*)dst_buf+bpos,cur->ptr+n_pos,n_rstcap);
            position+=n_rstcap;
            bpos+=n_rstcap;
            size-=n_rstcap;
            cur=cur->next;
            n_rstcap=cur->size;
            n_pos=0;
        }
    }
}
size_t ByteArray::getReadBuffs(std::vector<iovec>& dst_buffers,size_t len) const {
    len=len>getRestRdSize() ? getRestRdSize() : len;
    if(len==0){
        return 0;
    }

    size_t ans=len;

    size_t npos=m_position % m_NodeSize;
    size_t nrstcap=m_curr->size-npos;
    iovec iov;
    Node* cur=m_curr;

    while(len>0){
        memset(&iov,0,sizeof(iov));
        if(nrstcap>=len){
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=len;
            len=0;
        }else{
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=nrstcap;
            len-=nrstcap;
            cur=cur->next;
            npos=0;
            nrstcap=cur->size;
        }
        dst_buffers.push_back(iov);
    }
    return ans;
}
size_t ByteArray::getReadBuffs(std::vector<iovec>& dst_buffers,size_t len,size_t start_pos) const {
    len=len>getRestRdSize() ? getRestRdSize() : len;
    if(len==0 || start_pos>=m_position+getRestRdSize()){
        return 0;
    }

    size_t ans=len;

    size_t npos=start_pos % m_NodeSize;
    size_t ncount=start_pos / m_NodeSize;
    Node* cur=m_root;

    while(ncount>0){
        cur=cur->next;
        --ncount;
    }

    size_t nrstcap=cur->size-npos;
    iovec iov;

    while(len>0){
        memset(&iov,0,sizeof(iov));
        if(nrstcap>=len){
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=len;
            len=0;
        }else{
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=nrstcap;
            len-=nrstcap;
            cur=cur->next;
            npos=0;
            nrstcap=cur->size;
        }
        dst_buffers.push_back(iov);
    }
    return ans;
}
size_t ByteArray::getWriteBuffs(std::vector<iovec>& src_buffers,size_t len){
    if(len==0){
        return 0;
    }

    addCapacity(len);

    size_t ans=len;
    size_t npos=m_position % m_NodeSize;
    size_t nrstcap=m_curr->size-npos;
    iovec iov;
    Node* cur=m_curr;

    while(len>0){
        memset(&iov,0,sizeof(iov));
        if(nrstcap>=len){
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=len;
            len=0;
        }else{
            iov.iov_base=cur->ptr+npos;
            iov.iov_len=nrstcap;
            len-=nrstcap;
            cur=cur->next;
            nrstcap=cur->size;
            npos=0;
        }
        src_buffers.push_back(iov);
    }
    return ans;
}

void ByteArray::setPosition(size_t pos){
    if(pos>m_capacity){
        throw std::out_of_range("setPosition out of range");
    }
    m_position=pos;
    if(m_position>m_datsize){
        m_datsize=m_position;
    }
    m_curr=m_root;
    while(pos>=m_curr->size){
        pos-=m_curr->size;
        m_curr=m_curr->next;
    }
}
bool ByteArray::readToFile(const std::string& filename){
    std::ofstream ofs;
    ofs.open(filename,std::ios::trunc | std::ios::binary);
    if(!ofs){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"File open failed: "
                                        <<filename
                                        <<" errno: "<<errno<<" --- "<<strerror(errno);
        return false;
    }

    size_t read_size=getRestRdSize();
    size_t pos=m_position;
    Node* cur=m_curr;

    while(read_size>0){
        size_t npos=pos%m_NodeSize;
        size_t len=(read_size>m_NodeSize ? m_NodeSize : read_size)-npos;

        ofs.write(cur->ptr+npos,len);
        cur=cur->next;
        pos+=len;
        read_size-=len;
    }
    
    ofs.close();
    return true;
}
bool ByteArray::writeFromFile(const std::string& filename){
    std::ifstream ifs;
    ifs.open(filename,std::ios::binary);
    if(!ifs){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"File open failed: "
                                        <<filename
                                        <<" errno: "<<errno<<" --- "<<strerror(errno);
        return false;
    }

    // 用一个char数组从文件中读取数据
    std::shared_ptr<char> buff(new char[m_NodeSize],[](char* ptr){delete[] ptr;});

    while(!ifs.eof()){
        ifs.read(buff.get(),m_NodeSize);
        write(buff.get(),ifs.gcount());     // gcount()返回最后一个输入操作读取的字节数
    }

    ifs.close();
    return true;
}
void ByteArray::addCapacity(size_t size){
    size_t old_size=getRestCapacity();
    if(size==0 || old_size>=size){
        return;
    }

    size-=old_size;
    size_t n_count=ceil(1.0*size/m_NodeSize);       // 待新加的节点
    Node* tmp=m_root;
    while(tmp->next){
        tmp=tmp->next;
    }

    for(size_t i=0;i<n_count;++i){
        tmp->next=new Node(m_NodeSize);
        tmp=tmp->next;
        if(i==0 && old_size==0){
            m_curr=tmp;
        }
        m_capacity+=m_NodeSize;
    }
}

std::string ByteArray::toString() const{
    std::string s;
    s.resize(getRestRdSize());
    if(!s.empty()){
        read(&s[0],s.size(),m_position);
    }
    return s;
}
std::string ByteArray::toHexString() const{
    std::string str=toString();
    std::string hex="0123456789abcdef";
    std::string h_str;
    
    for(auto& c:str){
        h_str.push_back(hex[(c>>4) & 0xf]);     // &0xf 的意义是限制位数
        h_str.push_back(hex[c & 0xf]);
    }

    return h_str;
}
size_t ByteArray::getNodeCount() const {
    size_t n=0;
    Node* tmp=m_root;
    while(tmp){
        tmp=tmp->next;
        ++n;
    }
    return n;
}

}
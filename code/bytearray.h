#ifndef __MYWEB_BYTEARRAY_H__
#define __MYWEB_BYTEARRAY_H__

#include <memory>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace myWeb{

// 序列化类：可用于网络传输数据、或外存
class ByteArray
{
public:
    typedef std::shared_ptr<ByteArray> ptr;
    // 节点
    struct Node{
        Node(size_t s);
        Node();
        ~Node();

        char* ptr;              // 指向内存地址
        Node* next=nullptr;     // 指向下一节点
        size_t size;
    };

    // node_size：每块节点内存大小为 4Kb 
    ByteArray(size_t node_size=4096);
    ~ByteArray();

    // f--固定长度；v--可变长度(压缩字节，一个或两个字节压缩没有意义)
    // string: length:16/32/64 data 表示二进制流中先是sizeof(type)大小的表示string的长度，再是该长度的string data
    // write
    
    void writef_int8(int8_t val);
    void writef_uint8(uint8_t val);
    void writef_int16(int16_t val);
    void writef_uint16(uint16_t val);
    void writef_int32(int32_t val);
    void writef_uint32(uint32_t val);
    void writef_int64(int64_t val);
    void writef_uint64(uint64_t val);
    
    void writev_int32(int32_t val);
    void writev_uint32(uint32_t val);
    void writev_int64(int64_t val);
    void writev_uint64(uint64_t val);

    void write_float(float val);
    void write_double(double val);

    // length: int16 , data
    void writef_string_16(const std::string& val);
    // length: int32 , data
    void writef_string_32(const std::string& val);
    // length: int64 , data
    void writef_string_64(const std::string& val);
    // length: varint , data
    void writev_string(const std::string& val);
    // data
    void write_string_WithoutLength(const std::string& val);

    // read

    int8_t readf_int8();
    uint8_t readf_uint8();
    int16_t readf_int16();
    uint16_t readf_uint16();
    int32_t readf_int32();
    uint32_t readf_uint32();
    int64_t readf_int64();
    uint64_t readf_uint64();
    
    int32_t readv_int32();
    uint32_t readv_uint32();
    int64_t readv_int64();
    uint64_t readv_uint64();

    float read_float();
    double read_double();

    // length: int16 , data
    std::string readf_string_16();
    // length: int32 , data
    std::string readf_string_32();
    // length: int64 , data
    std::string readf_string_64();
    // length: varint , data
    std::string readv_string();

    // 只留根节点，释放其他节点
    void clear();

    // 获取字节序
    bool isLittleEndian() const ;
    // 设置字节序
    void setEndian(bool islittle);

    // 向节点中写入size长度的数据
    void write(const void* src_buf,size_t size);
    // 从节点中读取size长度的数据（使用前应现将m_position移至数据头位置）
    void read(void* dst_buf,size_t size);
    // 暂读，不改变m_position位置
    void read(void* dst_buf,size_t size,size_t position) const;

    // 将可读取的缓存转存为iovec数组，返回读取的字节长度
    size_t getReadBuffs(std::vector<iovec>& dst_buffers,size_t len=~0ul) const ;
    // 将可读取的缓存转存为iovec数组，返回读取的字节长度（指定起始位置）
    size_t getReadBuffs(std::vector<iovec>& dst_buffers,size_t len,size_t start_pos) const ;
    // 获取可写的缓存，转存为iovec数组，返回写入的字节长度
    size_t getWriteBuffs(std::vector<iovec>& src_buffers,size_t len);

    // 获取当前进行位置
    size_t getPosition() const {return m_position;}
    // 设置当前进行位置
    void setPosition(size_t pos);

    // 将节点内存中可读数据写入文件
    bool readToFile(const std::string& filename);
    // 从文件读数据到节点内存中
    bool writeFromFile(const std::string& filename);

    // 获取基础结点大小
    size_t getNodeSize() const {return m_NodeSize;}
    // 获取结点数量
    size_t getNodeCount() const ;
    // 获取剩余可读数据大小
    size_t getRestRdSize() const {return m_datsize-m_position;}
    // 获取当前数据大小
    size_t getSize() const {return m_datsize;}

    // 转为字符串
    std::string toString() const ;
    // 转为十六进制字符串
    std::string toHexString() const ;

private:
    // 扩容
    void addCapacity(size_t size);
    // 获取当前剩余容量
    size_t getRestCapacity() const {return m_capacity-m_position;}

private:
    // 每一个Node有多大
    size_t m_NodeSize;
    // 记录当前读（写）的位置（可写位置）
    size_t m_position;
    // 分配的总容量
    size_t m_capacity;
    // 当前已读（写）数据大小
    size_t m_datsize;
    // 字节序（默认大端）
    int8_t m_endian;

    Node* m_root;
    Node* m_curr;
};


}
#endif
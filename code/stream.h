#ifndef __MYWEB_STREAM_H__
#define __MYWEB_STREAM_H__

#include <memory>
#include "bytearray.h"

namespace myWeb{

/* 流（用来屏蔽文件和socket差异，封装 ByteArray ）
    以 socket 的角度进行读写
    socket recv————read
    socket send————write */
class Stream{
public:
    typedef std::shared_ptr<Stream> ptr;

    virtual ~Stream(){}

    // 读数据（buffer：接收数据的内存）
    virtual int read(void* buffer,size_t length)=0;
    // 读数据（ba：接收数据的ByteArray对象）
    virtual int read(ByteArray::ptr ba,size_t len)=0;
    // 写数据（buffer：数据来源）
    virtual int write(const void* buffer,size_t length)=0;
    // 写数据（ba：数据来源的ByteArray对象）
    virtual int write(ByteArray::ptr ba,size_t len)=0;

    // 读固定长度数据（buffer：接收数据的内存）
    virtual int read_Fix(void* buffer,size_t length);
    // 读固定长度数据（ba：接收数据的ByteArray对象）
    virtual int read_Fix(ByteArray::ptr ba,size_t len);
    // 写固定长度数据（buffer：数据来源）
    virtual int write_Fix(const void* buffer,size_t length);
    // 写固定长度数据（ba：数据来源的ByteArray对象）
    virtual int write_Fix(ByteArray::ptr ba,size_t len);

    // 关闭fd
    virtual void close()=0;

};

}
#endif

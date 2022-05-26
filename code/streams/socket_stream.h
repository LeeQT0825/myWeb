#ifndef __MYWEB_SOCKET_STREAM_H__
#define __MYWEB_SOCKET_STREAM_H__

#include "../stream.h"
#include "../socket.h"
#include "../iomanager.h"
#include "../mmutex.h"

namespace myWeb{

/* Socket 流(将 socketfd 作为文件 fd 读写，封装 ByteArray )
    socket recv————read
    socket send————write */
class Socket_Stream:public Stream{
public:
    typedef std::shared_ptr<Socket_Stream> ptr;

    // auto_destruct： 如果为true，this析构的时候会顺带关闭 sock 对象
    Socket_Stream(mySocket::ptr sock,bool auto_destruct=false);
    ~Socket_Stream();

    // 读数据（buffer：接收数据的内存）
    virtual int read(void* buffer,size_t length) override ;
    // 读数据（ba：接收数据的ByteArray对象）
    virtual int read(ByteArray::ptr ba,size_t len) override ;
    // 写数据（buffer：数据来源）
    virtual int write(const void* buffer,size_t length) override ;
    // 写数据（ba：数据来源的ByteArray对象）
    virtual int write(ByteArray::ptr ba,size_t len) override ;

    // 关闭socket
    virtual void close() override ;

    mySocket::ptr getSocket() const { return m_socket; }
    bool isConnected() const { return m_socket && m_socket->isConnected(); }

protected:
    mySocket::ptr m_socket;
    bool m_auto_destruct;

};


}
#endif
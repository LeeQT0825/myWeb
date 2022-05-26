#include "socket_stream.h"

namespace myWeb{

Socket_Stream::Socket_Stream(mySocket::ptr sock,bool auto_destruct)
                        :m_socket(sock)
                        ,m_auto_destruct(auto_destruct){}
Socket_Stream::~Socket_Stream(){
    if(m_socket && m_auto_destruct){
        m_socket->close();
    }
}

int Socket_Stream::read(void* buffer,size_t length){
    if(!isConnected())  return -1;
    return m_socket->recv(buffer,length);
}
int Socket_Stream::read(ByteArray::ptr ba,size_t length){
    if(!isConnected())  return -1;
    std::vector<iovec> iovs;
    ba->getWriteBuffs(iovs,length);
    int ret=m_socket->recv(&iovs[0],iovs.size());
    if(ret>0){
        ba->setPosition(ba->getPosition()+(size_t)ret);
    }

    return ret;
}
int Socket_Stream::write(const void* buffer,size_t length){
    if(!isConnected())  return -1;
    return m_socket->send(buffer,length);
}
int Socket_Stream::write(ByteArray::ptr ba,size_t length){
    if(!isConnected())  return -1;
    std::vector<iovec> iovs;
    ba->getReadBuffs(iovs,length);
    int ret=m_socket->send(&iovs[0],iovs.size());
    if(ret>0){
        ba->setPosition(ba->getPosition()+(size_t)ret);
    }

    return ret;
}
void Socket_Stream::close(){
    if(m_socket){
        m_socket->close();
    }
}


}
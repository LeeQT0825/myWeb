#ifndef __MYWEB_SOCKET_H__
#define __MYWEB_SOCKET_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <memory>
#include <ostream>
#include "address.h"

namespace myWeb{

// 套接字类
class mySocket:public std::enable_shared_from_this<mySocket>{
public:
    typedef std::shared_ptr<mySocket> ptr;
    typedef std::weak_ptr<mySocket> weak_ptr;

    mySocket(int family,int type,int protocol=0);
    virtual ~mySocket(){
        close();
    }

    // socket type
    enum sock_type{
        TCP=SOCK_STREAM,
        UDP=SOCK_DGRAM
    };

    // socket family
    enum sock_family{
        IPv4=AF_INET,
        IPv6=AF_INET6,
        UNIX=AF_UNIX
    };

    // 创建 TCP socket（根据地址类确定协议）
    static mySocket::ptr CreateTCPsocket(Address::ptr address);
    // 创建 TCP socket（IPv4）
    static mySocket::ptr CreateTCPsocket4();
    // 创建 TCP socket（IPv6）
    static mySocket::ptr CreateTCPsocket6();
    // 创建 UDP socket
    static mySocket::ptr CreateUDPsocket(Address::ptr address);
    // 创建 UDP socket（IPv4）
    static mySocket::ptr CreateUDPsocket4();
    // 创建 UDP socket（IPv6）
    static mySocket::ptr CreateUDPsocket6();

    // 获取发送超时时间
    uint64_t getSendTimeout();
    // 获取接收超时时间
    uint64_t getRecvTimeout();
    // 设置发送超时时间
    void setSendTimeout(uint64_t timeout_ms);
    // 设置发送超时时间
    void setRecvTimeout(uint64_t timeout_ms);
    // 获取socket选项
    bool getOption(int level,int opt,void* result,socklen_t* len);
    // 设置socket选项
    bool setOption(int level,int opt,const void* optval,socklen_t len);

    // 绑定地址
    virtual bool bind(const Address::ptr addr);
    // 监听
    virtual bool listen(int backlog=SOMAXCONN);
    // 接收socket连接
    virtual mySocket::ptr accept();
    // 连接地址（timeout_ms=-1时，禁用定时器）
    virtual bool connect(const Address::ptr addr,uint64_t timeout_ms=-1);
    // 重连
    virtual bool reconnect(uint64_t timeout_ms=-1);
    // 关闭
    virtual bool close();

    // TCP 写数据
    virtual int send(const void* src_buffer,size_t length,int flag=0);
    // TCP 写数据（集中写）
    virtual int send(const iovec* src_buffer,size_t iov_len,int flag=0);
    // UDP 写数据
    virtual int sendto(const void* src_buffer,size_t length,const Address::ptr dst,int flag=0);
    // UDP 写数据（集中写）
    virtual int sendto(const iovec* src_buffer,size_t iov_len,const Address::ptr dst,int flag=0);
    // TCP 读数据
    virtual int recv(void* dst_buffer,size_t length,int flag=0);
    // TCP 读数据（分散读）
    virtual int recv(iovec* dst_buffer,size_t iov_len,int flag=0);
    // UDP 读数据
    virtual int recvfrom(void* dst_buffer,size_t length,Address::ptr src,int flag=0);
    // UDP 读数据（分散读）
    virtual int recvfrom(iovec* dst_buffer,size_t iov_len,Address::ptr src,int flag=0);

    // 获取socket句柄
    int getSocketfd() const {
        return m_sockfd;
    }
    // 获取协议族
    int getFamily() const {
        return m_family;
    }
    // 获取套接字类型
    int getType() const {
        return m_type;
    }
    // 获取补充协议
    int getProtocol() const {
        return m_protocol;
    }
    // 获取本端地址
    Address::ptr getSrcAddr();
    // 获取对端地址
    Address::ptr getDstAddr();
    // 是否连接
    bool isConnected() const {
        return m_isConnected;
    }
    // 是否有效(m_socketfd!=-1)
    bool isValid() const ;
    // 返回 socket 错误
    int getError();

    // 转存信息
    virtual std::ostream& sockinfoDump(std::ostream& os) const ;
    // 输出字符串
    virtual std::string toString() const;

    // 取消读
    bool CancelRecv();
    // 取消写
    bool CancelSend();
    // 取消连接
    bool CancelAccept();
    // 取消全部事件
    bool CancelAll();

protected:
    // socket设置初始化
    void Init_setSocket();
    // 创建socket
    void newSocket();
    // 创建一个新的mySocket对象(accept返回一个新的connfd)时初始化
    virtual bool Init(int socket);

private:
    // 不可被复制
    mySocket(const mySocket& sock) =delete;
    mySocket& operator=(const mySocket& sock)=delete;

private:
    // socket句柄
    int m_sockfd;
    // 协议族
    int m_family;
    // 套接字类型
    int m_type;
    // 具体协议（默认为0）
    int m_protocol=0;
    // 连接状态（connfd：建立连接后为true； socketfd：创建即是true； AF_UNIX：默认为true）
    bool m_isConnected;
    // 本端地址
    Address::ptr m_srcaddr;
    // 对端地址
    Address::ptr m_dstaddr;

};


}
#endif
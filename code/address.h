#ifndef __MYWEB_ADDRESS_H__
#define __MYWEB_ADDRESS_H__

#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include "mmutex.h"

namespace myWeb{

// 所有网络地址的抽象类
class Address{
public:
    typedef std::shared_ptr<Address> ptr;
    virtual ~Address(){}
    
    // 获取协议族
    int getFamily();

    // 获取地址
    virtual const sockaddr* getAddr() const =0;
    // 获取地址长度
    virtual socklen_t getAddrlen() const =0;
    // 地址序列化（网络字节序转为可读序列）
    virtual std::ostream& insert(std::ostream& os);
    
    std::string toString();

    bool operator<(const Address& r) const;
    bool operator==(const Address& r) const;
    bool operator!=(const Address& r) const;

};

// IP协议相关地址抽象类
class IP_Address:public Address{
public:
    typedef std::shared_ptr<IP_Address> ptr;

    // 获取网关地址
    virtual IP_Address::ptr broadcastAddr(uint32_t prefix_len) =0;
    // 获取网段
    virtual IP_Address::ptr networdAddr(uint32_t prefix_len) =0;
    // 获取子网掩码
    virtual IP_Address::ptr subnetMask(uint32_t prefix_len) =0;
    // 返回端口号
    virtual uint16_t getPort() const =0;
    // 设置端口号
    virtual void setPort(uint16_t port) =0;

};

// IPv4 地址类
class IPv4_Address:public IP_Address{
public:
    typedef std::shared_ptr<IPv4_Address> ptr;
    // 通过二进制地址 address 构造,传入的都为小端序列
    IPv4_Address(uint32_t address=INADDR_ANY,uint16_t port=0);
    IPv4_Address(const sockaddr_in& address);

    // 获取地址
    const sockaddr* getAddr() const override;
    // 获取地址长度
    socklen_t getAddrlen() const override;
    // 地址序列化（网络字节序转为可读序列）
    std::ostream& insert(std::ostream& os) override;
    // 获取网关地址
    IP_Address::ptr broadcastAddr(uint32_t prefix_len) override;
    // 获取网段
    IP_Address::ptr networdAddr(uint32_t prefix_len) override;
    // 获取子网掩码
    IP_Address::ptr subnetMask(uint32_t prefix_len) override;
    // 返回端口号
    uint16_t getPort() const override;
    // 设置端口号
    void setPort(uint16_t port) override;

private:
    // 网络字节序
    sockaddr_in m_addr;
};

// IPv6 地址类
class IPv6_Address:public IP_Address{
public:
    typedef std::shared_ptr<IPv6_Address> ptr;
    IPv6_Address();
    // 用16字节字符串构造，传入的都为小端序列
    IPv6_Address(const char* address,uint16_t port);

    // 获取地址
    const sockaddr* getAddr() const override;
    // 获取地址长度
    socklen_t getAddrlen() const override;
    // 地址序列化（网络字节序转为可读序列）
    std::ostream& insert(std::ostream& os) override;
    // 获取网关地址
    IP_Address::ptr broadcastAddr(uint32_t prefix_len) override;
    // 获取网段
    IP_Address::ptr networdAddr(uint32_t prefix_len) override;
    // 获取子网掩码
    IP_Address::ptr subnetMask(uint32_t prefix_len) override;
    // 返回端口号
    uint16_t getPort() const override;
    // 设置端口号
    void setPort(uint16_t port) override;

private:
    sockaddr_in6 m_addr;
};

// Unix 地址类
// class Unix_Address:public Address{
// public:
//     typedef std::shared_ptr<Unix_Address> ptr;
//     Unix_Address(const std::string& address);

//     // 获取地址
//     const sockaddr* getAddr() const override;
//     // 获取地址长度
//     socklen_t getAddrlen() const override;
//     // 地址序列化
//     std::ostream& insert(std::ostream& os) const override;

// private:
//     sockaddr_un m_addr;
//     socklen_t m_length;
// };

}
#endif
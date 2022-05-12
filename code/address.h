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

    // 已知地址结构，创建地址
    static Address::ptr Create_addr(const sockaddr* address,socklen_t len);
    // 根据主机名、服务名(或字符串表示的地址)创建地址
    static bool Lookup(std::vector<Address::ptr>& addrs,const std::string& hostname,
                int family=AF_INET, int type=0,int protocol=0);
    // 根据网卡获取地址
    static bool getInterfaceAddress(std::multimap<std::string,std::pair<Address::ptr,uint32_t> >& result,
                int family=AF_INET);
    // 获取协议族
    int getFamily() const;

    // 获取地址
    virtual const sockaddr* getAddr() const =0;
    // 获取地址长度
    virtual socklen_t getAddrlen() const =0;
    // 地址序列化（网络字节序转为可读序列）
    virtual std::ostream& insert(std::ostream& os) =0;
    
    std::string toString();

    bool operator<(const Address& r) const;
    bool operator==(const Address& r) const;
    bool operator!=(const Address& r) const;

};

// IP协议相关地址抽象类
class IP_Address:public Address{
public:
    typedef std::shared_ptr<IP_Address> ptr;

    /* 创建地址(获取本地地址信息) 
        目前仅支持IPv4，后期可以修改 hints.ai_family 参数 */
    static IP_Address::ptr Create_addr(const char* address,uint16_t port=0);

    // 获取广播地址
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

    // 创建IPv4地址类接口
    static IPv4_Address::ptr Create_addr(const char* address,uint16_t port);

    // 获取地址
    const sockaddr* getAddr() const override;
    // 获取地址长度
    socklen_t getAddrlen() const override;
    // 地址序列化（网络字节序转为可读序列）
    std::ostream& insert(std::ostream& os) override;
    // 获取广播地址
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

// // IPv6 地址类
// class IPv6_Address:public IP_Address{
// public:
//     typedef std::shared_ptr<IPv6_Address> ptr;
//     IPv6_Address();
//     // 用字符串构造，传入的都为小端序列
//     IPv6_Address(const char* address,uint16_t port);

//     // 创建IPv6地址类接口
//     static IPv6_Address::ptr Create_addr(const char* address,uint16_t port);

//     // 获取地址
//     const sockaddr* getAddr() const override;
//     // 获取地址长度
//     socklen_t getAddrlen() const override;
//     // 地址序列化（网络字节序转为可读序列）
//     std::ostream& insert(std::ostream& os) override;
//     // 获取广播地址(prefix_len 掩码位数)
//     IP_Address::ptr broadcastAddr(uint32_t prefix_len) override;
//     // 获取网段
//     IP_Address::ptr networdAddr(uint32_t prefix_len) override;
//     // 获取子网掩码
//     IP_Address::ptr subnetMask(uint32_t prefix_len) override;
//     // 返回端口号
//     uint16_t getPort() const override;
//     // 设置端口号
//     void setPort(uint16_t port) override;

// private:
//     sockaddr_in6 m_addr;
// };

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
#include "address.h"
#include <sstream>

namespace myWeb{

// Address类

int Address::getFamily(){
    return getAddr()->sa_family;
}
std::string Address::toString(){
    std::stringstream ss;
    insert(ss);
    return ss.str();
}
bool Address::operator<(const Address& r) const{
    // 优先比较字符，再比长度
    socklen_t minlen=std::min(getAddrlen(),r.getAddrlen());
    int ans=memcmp(getAddr(),r.getAddr(),minlen);
    if(ans<0){
        return true;
    }else if(ans>0){
        return false;
    }else if(getAddrlen()<r.getAddrlen()){
        return true;
    }
    return false;
}
bool Address::operator==(const Address& r) const{
    if(getAddrlen()!=r.getAddrlen()){
        return false;
    }
    int ans=memcmp(getAddr(),r.getAddr(),getAddrlen());
    if(ans==0){
        return true;
    }
    return false;
}
bool Address::operator!=(const Address& r) const{
    if(getAddrlen()!=r.getAddrlen()){
        return true;
    }
    int ans=memcmp(getAddr(),r.getAddr(),getAddrlen());
    if(ans==0){
        return false;
    }
    return true;
}

// IPv4_Address类

IPv4_Address::IPv4_Address(uint32_t address=INADDR_ANY,uint16_t port=0){
    memset(&m_addr,0,sizeof(sockaddr));
    m_addr.sin_family=AF_INET;
    m_addr.sin_port=htons(port);
    m_addr.sin_addr.s_addr=htonl(address);      
}
IPv4_Address::IPv4_Address(const sockaddr_in& address){
    m_addr=address;
}
const sockaddr* IPv4_Address::getAddr() const{
    return (const sockaddr*)&m_addr;
}
socklen_t IPv4_Address::getAddrlen() const{
    return sizeof(m_addr);
}
std::ostream& IPv4_Address::insert(std::ostream& os){
    char* address;
    inet_ntop(AF_INET,&m_addr.sin_addr,address,INET_ADDRSTRLEN);
    os<<address;    
    os<<":"<<ntohs(m_addr.sin_port);
}
IP_Address::ptr IPv4_Address::broadcastAddr(uint32_t prefix_len){

}
IP_Address::ptr IPv4_Address::networdAddr(uint32_t prefix_len){

}
IP_Address::ptr IPv4_Address::subnetMask(uint32_t prefix_len){

}
uint16_t IPv4_Address::getPort() const{
    return ntohs(m_addr.sin_port);
}
void IPv4_Address::setPort(uint16_t port){
    m_addr.sin_port=htons(port);
}


// IPv6_Address类

IPv6_Address::IPv6_Address(){}
IPv6_Address::IPv6_Address(const char* address,uint16_t port){
    memset(&m_addr,0,sizeof(sockaddr));
    m_addr.sin6_family=AF_INET6;
    m_addr.sin6_port=htons(port);
    inet_pton(AF_INET6,address,&m_addr.sin6_addr.__in6_u);  
}
const sockaddr* IPv6_Address::getAddr() const{
    return (const sockaddr*)&m_addr;
}
socklen_t IPv6_Address::getAddrlen() const{
    return sizeof(m_addr);
}
std::ostream& IPv6_Address::insert(std::ostream& os){
    char* address;
    inet_ntop(AF_INET6,&m_addr.sin6_addr,address,INET6_ADDRSTRLEN); 
    os<<"["<<address<<"]";    // 不可重入
    os<<":"<<ntohs(m_addr.sin6_port);
}
IP_Address::ptr IPv6_Address::broadcastAddr(uint32_t prefix_len){

}
IP_Address::ptr IPv6_Address::networdAddr(uint32_t prefix_len){

}
IP_Address::ptr IPv6_Address::subnetMask(uint32_t prefix_len){

}
uint16_t IPv6_Address::getPort() const{
    return ntohs(m_addr.sin6_port);
}
void IPv6_Address::setPort(uint16_t port){
    m_addr.sin6_port=htons(port);
}

}
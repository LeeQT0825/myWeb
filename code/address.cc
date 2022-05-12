#include "address.h"
#include "log.h"
#include <string.h>
#include <sstream>
#include <netdb.h>
#include <ifaddrs.h>

namespace myWeb{

// 获取子网掩码(大端),bits：掩码位数
template<typename T>
static T CreateMask(uint32_t bits){
    return (1<<bits)-1;
}

// 计算二进制数中有多少个1
template<typename T>
static uint32_t CountBytes(T value){
    uint32_t count=0;
    while(value){
        ++count;
        value &= value-1;
    }
    return count;
}

// Address类

Address::ptr Address::Create_addr(const sockaddr* address,socklen_t len){
    if(address==nullptr){
        return nullptr;
    }
    IP_Address::ptr ad;
    switch (address->sa_family)
    {
    case AF_INET:
        ad.reset(new IPv4_Address(*(const sockaddr_in*)address));
        break;
    
    default:
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Not Support yet";
        return nullptr;
        break;
    }
    return ad;
    
}
bool Address::Lookup(std::vector<Address::ptr>& addrs,
                const std::string& hostname,int family, int type,int protocol){
    addrinfo hints,*result,*next;
    bzero(&hints,sizeof(addrinfo));
    hints.ai_family=family;
    hints.ai_socktype=type;
    hints.ai_protocol=protocol;
    hints.ai_canonname=NULL;
    hints.ai_addr=NULL;
    hints.ai_next=NULL;

    std::string node;                       // 服务名称
    const char* service=nullptr;            // 服务名称字符串

    // IPv6
    if(!hostname.empty() && hostname[0]=='['){
        const char* endipv6=(const char*)memchr(hostname.c_str()+1,']',hostname.size()-1);
        if(endipv6){
            if(*(endipv6+1)==':'){
                service=endipv6+2;
            }
            node=hostname.substr(1,endipv6-hostname.c_str()-1);
        }
    }

    // 检查 IPv4 service
    if(node.empty()){
        service=(const char*)memchr(hostname.c_str(),':',hostname.size());
        if(service){
            if(!memchr(service+1,':',hostname.c_str()+hostname.size()-1-service)){
                node=hostname.substr(0,service-hostname.c_str());
                ++service;
            }
        }
    }

    if(node.empty()){
        node=hostname;
    }

    int error=getaddrinfo(hostname.c_str(),service,&hints,&result);
    if(error){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Address Lookup(): errno: "<<errno<<" --- "<<strerror(errno);
        return false;
    }

    // 根据result链表创建地址
    next=result;
    while(next){
        addrs.push_back(Create_addr(next->ai_addr,next->ai_addrlen));
        next=next->ai_next;
    }

    freeaddrinfo(result);
    return !addrs.empty();
    
}
bool Address::getInterfaceAddress(std::multimap<std::string,std::pair<Address::ptr,uint32_t> >& result,
                int family){
    ifaddrs *results,*next;
    if(getifaddrs(&results)!=0){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"Address getInterfaceAddress(): errno: "<<errno<<" --- "<<strerror(errno);
        return false;
    }

    try{
        for(next=results;next;next=next->ifa_next){
            Address::ptr addr;
            uint32_t prefix_len=~0u;
            if(family!=AF_UNSPEC && family!=next->ifa_addr->sa_family){
                continue;
            }
            switch (next->ifa_addr->sa_family)
            {
            case AF_INET:
                {
                    addr=Create_addr(next->ifa_addr,sizeof(sockaddr_in));
                    uint32_t net_mask=((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                    prefix_len=CountBytes(net_mask);
                }
                break;
            case AF_INET6:
                // TODO
                break;
            default:
                break;
            }

            if(addr){
                result.insert(std::make_pair(next->ifa_name,std::make_pair(addr,prefix_len)));
            }
        }
    }catch(...){
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return true;
}
int Address::getFamily() const{
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

// IP_Address 类

IP_Address::ptr IP_Address::Create_addr(const char* address,uint16_t port){
    addrinfo hints;
    addrinfo* result;
    bzero(&hints,sizeof(addrinfo));

    hints.ai_family=AF_INET;
    hints.ai_flags=AI_NUMERICHOST;      // 指定hostname必须是字符串表示的IP地址，避免DNS查询

    int err=getaddrinfo(address,NULL,&hints,&result);
    if(err){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"IP Create_addr( "<<address<<" , "
                                            <<port<<" ): errno: "<<errno<<" --- "<<strerror(errno);
        return nullptr;
    }

    try{
        IP_Address::ptr res=std::dynamic_pointer_cast<IP_Address>(
                                        Address::Create_addr(result->ai_addr,(socklen_t)result->ai_addrlen));
        if(res){
            res->setPort(port);
        }
        freeaddrinfo(result);
        return res;
    }catch(...){
        freeaddrinfo(result);
        return nullptr;
    }
}

// IPv4_Address类

IPv4_Address::ptr IPv4_Address::Create_addr(const char* address,uint16_t port){
    IPv4_Address::ptr ad(new IPv4_Address());
    ad->m_addr.sin_port=htons(port);
    int ret=inet_pton(AF_INET,address,&ad->m_addr.sin_addr);
    if(ret<=0){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"IPv4 Create_addr( "<<address<<" , "
                                            <<port<<" ): errno: "<<errno<<" --- "<<strerror(errno);
        return nullptr;
    }
    return ad;
}
IPv4_Address::IPv4_Address(uint32_t address,uint16_t port){
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
    char address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&m_addr.sin_addr,address,INET_ADDRSTRLEN);    // 可重入
        os<<address;    
        os<<":"<<ntohs(m_addr.sin_port);
    return os;
}
IP_Address::ptr IPv4_Address::broadcastAddr(uint32_t prefix_len){
    if(prefix_len>32){
        return nullptr;
    }
    sockaddr_in broadaddr(m_addr);
    broadaddr.sin_addr.s_addr |= (~CreateMask<uint32_t>(prefix_len));
    return IPv4_Address::ptr(new IPv4_Address(broadaddr));
}
IP_Address::ptr IPv4_Address::networdAddr(uint32_t prefix_len){
    if(prefix_len>32){
        return nullptr;
    }
    sockaddr_in broadaddr(m_addr);
    broadaddr.sin_addr.s_addr &= CreateMask<uint32_t>(prefix_len);
    return IPv4_Address::ptr(new IPv4_Address(broadaddr));  
}
IP_Address::ptr IPv4_Address::subnetMask(uint32_t prefix_len){
    sockaddr_in subnet_addr;
    memset(&subnet_addr,0,sizeof(sockaddr_in));
    subnet_addr.sin_family=AF_INET;
    subnet_addr.sin_addr.s_addr=CreateMask<uint32_t>(prefix_len);
    return IPv4_Address::ptr(new IPv4_Address(subnet_addr));
}
uint16_t IPv4_Address::getPort() const{
    return ntohs(m_addr.sin_port);
}
void IPv4_Address::setPort(uint16_t port){
    m_addr.sin_port=htons(port);
}


// IPv6_Address类

// IPv6_Address::ptr IPv6_Address::Create_addr(const char* address,uint16_t port){

// }
// IPv6_Address::IPv6_Address(){}
// IPv6_Address::IPv6_Address(const char* address,uint16_t port){
//     memset(&m_addr,0,sizeof(sockaddr));
//     m_addr.sin6_family=AF_INET6;
//     m_addr.sin6_port=htons(port);
//     inet_pton(AF_INET6,address,&m_addr.sin6_addr.__in6_u);  
// }
// const sockaddr* IPv6_Address::getAddr() const{
//     return (const sockaddr*)&m_addr;
// }
// socklen_t IPv6_Address::getAddrlen() const{
//     return sizeof(m_addr);
// }
// std::ostream& IPv6_Address::insert(std::ostream& os){
    // char address[INET6_ADDRSTRLEN];
//     inet_ntop(AF_INET6,&m_addr.sin6_addr,address,INET6_ADDRSTRLEN);     // 可重入
//     os<<"["<<address<<"]";    
//     os<<":"<<ntohs(m_addr.sin6_port);
// }
// IP_Address::ptr IPv6_Address::broadcastAddr(uint32_t prefix_len){
//     sockaddr_in6 broadaddr;
//     broadaddr.sin6_addr.__in6_u.__u6_addr8[prefix_len/8]=CreateMask<uint8_t>(prefix_len/8);
// }
// IP_Address::ptr IPv6_Address::networdAddr(uint32_t prefix_len){

// }
// IP_Address::ptr IPv6_Address::subnetMask(uint32_t prefix_len){

// }
// uint16_t IPv6_Address::getPort() const{
//     return ntohs(m_addr.sin6_port);
// }
// void IPv6_Address::setPort(uint16_t port){
//     m_addr.sin6_port=htons(port);
// }

}
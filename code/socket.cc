#include "socket.h"
#include "fdmanager.h"
#include "iomanager.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace myWeb{

mySocket::ptr mySocket::CreateTCPsocket(Address::ptr address){
    mySocket::ptr sock(new mySocket(address->getFamily(),TCP));
    return sock;
}
mySocket::ptr mySocket::CreateTCPsocket4(){
    mySocket::ptr sock(new mySocket(IPv4,TCP));
    return sock;
}
mySocket::ptr mySocket::CreateTCPsocket6(){
    mySocket::ptr sock(new mySocket(IPv6,TCP));
    return sock;
}
mySocket::ptr mySocket::CreateUDPsocket(Address::ptr address){
    mySocket::ptr sock(new mySocket(address->getFamily(),UDP));
    sock->newSocket();
    sock->m_isConnected=true;
    return sock;
}
mySocket::ptr mySocket::CreateUDPsocket4(){
    mySocket::ptr sock(new mySocket(IPv4,TCP));
    sock->newSocket();
    sock->m_isConnected=true;
    return sock;
}
mySocket::ptr mySocket::CreateUDPsocket6(){
    mySocket::ptr sock(new mySocket(IPv6,TCP));
    sock->newSocket();
    sock->m_isConnected=true;
    return sock;
}

mySocket::mySocket(int family,int type,int protocol)
            :m_sockfd(-1)
            ,m_family(family)
            ,m_type(type)
            ,m_protocol(protocol)
            ,m_isConnected(false){}
uint64_t mySocket::getSendTimeout(){
    FDctx::ptr fd_ctx=Singleton<FDManager>::getInstance()->getFD(m_sockfd);
    if(fd_ctx && fd_ctx->isSocket() && !fd_ctx->isClosed()){
        return fd_ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}
uint64_t mySocket::getRecvTimeout(){
    FDctx::ptr fd_ctx=Singleton<FDManager>::getInstance()->getFD(m_sockfd);
    if(fd_ctx && fd_ctx->isSocket() && !fd_ctx->isClosed()){
        return fd_ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}
void mySocket::setSendTimeout(uint64_t timeout_ms){
    timeval timeo{int(timeout_ms/1000),int((timeout_ms%1000)*1000)};
    setOption(SOL_SOCKET,SO_SNDTIMEO,&timeo,sizeof(timeo));
}
void mySocket::setRecvTimeout(uint64_t timeout_ms){
    timeval timeo{int(timeout_ms/1000),int((timeout_ms%1000)*1000)};
    setOption(SOL_SOCKET,SO_RCVTIMEO,&timeo,sizeof(timeo));
}
bool mySocket::getOption(int level,int opt,void* result,socklen_t* len){
    if(::getsockopt(m_sockfd,level,opt,result,len)){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"getOption(): errno: "<<errno<<" --- "<<strerror(errno);
        return false;
    }
    return true;
}
bool mySocket::setOption(int level,int opt,const void* optval,socklen_t len){
    if(::setsockopt(m_sockfd,level,opt,optval,len)){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"setOption(): errno: "<<errno<<" --- "<<strerror(errno);
        return false;
    }
    return true;
}
Address::ptr mySocket::getSrcAddr(){
    if(m_srcaddr){
        return m_srcaddr;
    }

    Address::ptr addr;
    switch (m_family)
    {
    case AF_INET:
        addr.reset(new IPv4_Address());
        break;
    case AF_INET6:
    case AF_UNIX:
    default:
        break;
    }
    if(addr){
        sockaddr* sock_addr=(sockaddr*)addr->getAddr();
        socklen_t sock_len=sizeof(sockaddr);
        if(getsockname(m_sockfd,sock_addr,&sock_len)){
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"getsockname(): errno: "<<errno<<" --- "<<strerror(errno);
            return nullptr;
        }
        m_srcaddr=addr;
    }
    
    return m_srcaddr;
}
Address::ptr mySocket::getDstAddr(){
    if(m_dstaddr){
        return m_dstaddr;
    }

    Address::ptr addr;
    switch (m_family)
    {
    case AF_INET:
        addr.reset(new IPv4_Address());
        break;
    case AF_INET6:
    case AF_UNIX:
    default:
        break;
    }
    if(addr){
        sockaddr* peer_sock=(sockaddr*)addr->getAddr();
        socklen_t peer_len=sizeof(sockaddr);
        if(getpeername(m_sockfd,peer_sock,&peer_len)){
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"getpeername(): errno: "<<errno<<" --- "<<strerror(errno);
            return nullptr;
        }
        m_dstaddr=addr;
    }

    return m_dstaddr;
}
bool mySocket::bind(const Address::ptr addr){
    // 检查 m_sockfd 是否合法
    if(!isValid()){
        newSocket();
        if(!isValid()){
            return false;
        }
    }
    // 检查 m_family 是否一致
    if(addr->getFamily()!=m_family){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"::bind() sockaddr_in.so_family is invalid";
        return false;
    }

    // 绑定
    int ret=::bind(m_sockfd,addr->getAddr(),addr->getAddrlen());
    if(ret==-1){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"::bind(): errno: "<<errno<<" --- "<<strerror(errno);
        return false;
    }
    getSrcAddr();
    return true;
}
bool mySocket::listen(int backlog){
    // 检查 m_sockfd 是否合法
    if(!isValid()){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"::listen(): error: sockfd=-1";
        return false;
    }
    // 监听
    int ret=::listen(m_sockfd,backlog);
    if(ret==-1){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"::listen(): errno: "<<errno<<" --- "<<strerror(errno);
        return false;
    }
    return true;
}
mySocket::ptr mySocket::accept(){
    mySocket::ptr conn_socket(new mySocket(IPv4,TCP));
    // 这里对对端地址表示不关心，所以置位nullptr，后面在 Init() 的时候会重新获取对端地址
    int connfd=::accept(m_sockfd,nullptr,nullptr);
    if(connfd==-1){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"::accept(): errno: "<<errno<<" --- "<<strerror(errno);
        return nullptr;
    }
    if(conn_socket->Init(connfd)){
        return conn_socket;
    }
    return nullptr;
}
bool mySocket::connect(const Address::ptr addr,uint64_t timeout_ms){
    // 检查 m_sockfd 是否合法
    if(!isValid()){
        newSocket();
        if(!isValid()){
            return false;
        }
    }
    // 检查 m_family 是否一致
    if(addr->getFamily()!=m_family){
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"::bind() sockaddr_in.so_family is invalid";
        return false;
    }

    m_dstaddr=addr;
    if(timeout_ms==(uint64_t)-1){
        if(::connect(m_sockfd,addr->getAddr(),addr->getAddrlen())){
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"::connect(): errno: "<<errno<<" --- "<<strerror(errno);
            close();
            return false;
        }
    }else{
        if(::connect_timeout(m_sockfd,addr->getAddr(),addr->getAddrlen(),timeout_ms)){
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"::connect(): errno: "<<errno<<" --- "<<strerror(errno);
            close();
            return false;
        }
    }

    m_isConnected=true;
    getSrcAddr();
    getDstAddr();
    return true;
}
bool mySocket::reconnect(uint64_t timeout_ms){
    if(!m_dstaddr){
        INLOG_DEBUG(MYWEB_NAMED_LOG("system"))<<"reconnect is null";
        return false;
    }
    m_srcaddr.reset();
    return connect(m_dstaddr,timeout_ms);
}
bool mySocket::close(){
    if(m_sockfd==-1 && !m_isConnected){
        return true;
    }
    m_isConnected=false;
    if(m_sockfd!=-1){
        ::close(m_sockfd);
        m_sockfd=-1;
    }
    return true;
}
int mySocket::send(const void* buffer,size_t length,int flag){
    if(isConnected()){
        return ::send(m_sockfd,buffer,length,flag);
    }
    return -1;
}
int mySocket::send(const iovec* buffers,size_t iov_len,int flag){
    if(isConnected()){
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov=(iovec*)buffers;
        msg.msg_iovlen=iov_len;
        return ::sendmsg(m_sockfd,&msg,flag);
    }
    return -1;
}
int mySocket::sendto(const void* buffer,size_t length,const Address::ptr dst,int flag){
    if(isConnected()){
        return ::sendto(m_sockfd,buffer,length,flag,dst->getAddr(),dst->getAddrlen());
    }
    return -1;
}
int mySocket::sendto(const iovec* buffers,size_t iov_len,const Address::ptr dst,int flag){
    if(isConnected()){
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov=const_cast<iovec*>(buffers);
        msg.msg_iovlen=iov_len;
        msg.msg_name=dst->getAddr();
        msg.msg_namelen=dst->getAddrlen();
        return ::sendmsg(m_sockfd,&msg,flag);
    }
    return -1;
}
int mySocket::recv(void* buffer,size_t length,int flag){
    if(isConnected()){
        return ::recv(m_sockfd,buffer,length,flag);
    }
    return -1;
}
int mySocket::recv(iovec* buffers,size_t iov_len,int flag){
    if(isConnected()){
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov=(iovec*)buffers;
        msg.msg_iovlen=iov_len;
        return ::recvmsg(m_sockfd,&msg,flag);
    }
    return -1;
}
int mySocket::recvfrom(void* buffer,size_t length,Address::ptr src,int flag){
    if(isConnected()){
        socklen_t addr_len=src->getAddrlen();
        return ::recvfrom(m_sockfd,buffer,length,flag,src->getAddr(),&addr_len);
    }
    return -1;
}
int mySocket::recvfrom(iovec* buffers,size_t iov_len,Address::ptr src,int flag){
    if(isConnected()){
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iov=(iovec*)buffers;
        msg.msg_iovlen=iov_len;
        msg.msg_name=src->getAddr();
        msg.msg_namelen=src->getAddrlen();
        return ::sendmsg(m_sockfd,&msg,flag);
    }
    return -1;
}
bool mySocket::isValid() const {
    return m_sockfd==-1 ? false : true;
}
int mySocket::getError(){
    int error=0;
    socklen_t len=sizeof(error);
    if(!getOption(SOL_SOCKET,SO_ERROR,&error,&len)){
        error=errno;
    }
    return error;
}
std::ostream& mySocket::sockinfoDump(std::ostream& os) const {
    os  <<"\n[Socket sock= "<<m_sockfd
        <<"\n is_Connected= "<<m_isConnected
        <<"\n family= "<<m_family
        <<"\n type= "<<m_type
        <<"\n protocol= "<<m_protocol;
    if(m_srcaddr){
        os<<"\n src_addr: "<<m_srcaddr->toString();
    }
    if(m_dstaddr){
        os<<"\n dst_addr: "<<m_dstaddr->toString();
    }
    os<<"]";
    return os;
}
std::string mySocket::toString() const{
    std::stringstream ss;
    sockinfoDump(ss);
    return ss.str();
}
bool mySocket::CancelRecv(){
    return IOManager::getThis()->cancelEvent(m_sockfd,IOManager::Event::READ);
}
bool mySocket::CancelSend(){
    return IOManager::getThis()->cancelEvent(m_sockfd,IOManager::Event::WRITE);
}
bool mySocket::CancelAccept(){
    return IOManager::getThis()->cancelEvent(m_sockfd,IOManager::Event::READ);
}
bool mySocket::CancelAll(){
    return IOManager::getThis()->cancelAllEvent(m_sockfd);
}
void mySocket::Init_setSocket(){
    int val=1;
    // 地址复用（无 TIME_WAIT ）
    setOption(SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
    if(m_type==TCP){
        // 禁用Nagle算法(应当在accept之前)
        setOption(IPPROTO_TCP,TCP_NODELAY,&val,sizeof(val));
    }
}
void mySocket::newSocket(){
    m_sockfd=socket(m_family,m_type,m_protocol);
    if(m_sockfd != -1){
        Init_setSocket();
    }else{
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"socket(): errno: "<<errno<<" --- "<<strerror(errno);
    }
}
bool mySocket::Init(int socket){
    FDctx::ptr fd_ctx=Singleton<FDManager>::getInstance()->getFD(socket);
    if(fd_ctx && fd_ctx->isSocket() && !fd_ctx->isClosed()){
        m_sockfd=socket;
        m_isConnected=true;
        Init_setSocket();
        m_srcaddr=getSrcAddr();
        m_dstaddr=getDstAddr();
        return true;
    }
    return false;
}

}
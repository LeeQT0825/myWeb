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
    mySocket::ptr sock(new mySocket(AF_INET,TCP));
    return sock;
}
mySocket::ptr mySocket::CreateTCPsocket6(){
    mySocket::ptr sock(new mySocket(AF_INET6,TCP));
    return sock;
}
mySocket::ptr mySocket::CreateUDPsocket(Address::ptr address){
    mySocket::ptr sock(new mySocket(address->getFamily(),UDP));
    sock->newSocket();
    sock->m_isConnected=true;
    return sock;
}
mySocket::ptr mySocket::CreateUDPsocket4(){

}
mySocket::ptr mySocket::CreateUDPsocket6(){

}

mySocket::mySocket(int family,int type,int protocol)
            :m_sockfd(-1)
            ,m_family(family)
            ,m_type(type)
            ,m_protocol(protocol)
            ,m_isConnected(false){}
uint64_t mySocket::getSendTimeout(){

}
uint64_t mySocket::getRecvTimeout(){

}
void mySocket::setSendTimeout(uint64_t timeout){

}
void mySocket::setRecvTimeout(uint64_t timeout){

}
bool mySocket::getOption(int level,int opt,void* result,socklen_t* len){

}
bool mySocket::setOption(int level,int opt,const void* optval,socklen_t len){

}
bool mySocket::bind(const Address::ptr addr){

}
bool mySocket::listen(int backlog=SOMAXCONN){

}
mySocket::ptr mySocket::accept(){

}
bool mySocket::connect(const Address::ptr addr,uint64_t timeout_ms=-1){

}
bool mySocket::reconnect(uint64_t timeout_ms=-1){

}
bool mySocket::close(){

}
int mySocket::send(const void* buffer,size_t length,int flag=0){

}
int mySocket::send(const iovec* buffers,size_t iov_len,int flad=0){

}
int mySocket::sendto(const void* buffer,size_t length,const Address::ptr dst,int flag=0){

}
int mySocket::sendto(const iovec* buffers,size_t iov_len,const Address::ptr dst,int flad=0){

}
int mySocket::recv(void* buffer,size_t length,int flag=0){

}
int mySocket::recv(iovec* buffers,size_t iov_len,int flad=0){

}
int mySocket::recvfrom(void* buffer,size_t length,Address::ptr src,int flag=0){

}
int mySocket::recvfrom(iovec* buffers,size_t iov_len,Address::ptr src,int flad=0){

}
bool mySocket::isValid() const {

}
int mySocket::getError(){

}
std::ostream& mySocket::sockDump(std::ostream& os) const {

}
std::string mySocket::toString() const{

}
bool mySocket::CancelRecv(){

}
bool mySocket::CancelSend(){

}
bool mySocket::CancelAccept(){

}
bool mySocket::CancelAll(){

}
void mySocket::InitSocket(){
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
        InitSocket();
    }else{
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"socket(): errno: "<<errno<<" --- "<<strerror(errno);
    }
}
bool mySocket::Init(int socket){

}

}
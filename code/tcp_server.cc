#include "tcp_server.h"

namespace myWeb{

static ConfigVar<uint64_t>::ptr config_tcp_server_recv_timeout
                                    =Config::Lookup("tcp_server.recv_timeout"
                                                ,(uint64_t)(1000*60*2),"tcp_server_recv_timeout");

TCP_Server::TCP_Server(IOManager* worker)
                    :m_worker(worker)
                    ,m_recvTimeout(config_tcp_server_recv_timeout->getVal())
                    ,m_name("myWeb_TCP/1.0.0")
                    ,m_isStop(true){}
TCP_Server::~TCP_Server(){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"TCP_Server destructed";
    stop();
}

bool TCP_Server::bind_listen(Address::ptr addr){
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> f_addrs;
    addrs.push_back(addr);
    return bind_listen(addrs,f_addrs);
}
bool TCP_Server::bind_listen(const std::vector<Address::ptr>& addrs,std::vector<Address::ptr>& f_addrs){
    for(auto& addr:addrs){
        mySocket::ptr sock=mySocket::CreateTCPsocket(addr);
        if(!sock->bind(addr)){
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"TCP_Server bind("<<addr->toString()<<") ERROR";
            f_addrs.push_back(addr);
            continue;
        }
        if(!sock->listen()){
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"TCP_Server listen() ERROR: "<<addr->toString();
            f_addrs.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }
    if(!f_addrs.empty()){
        m_socks.clear();
        return false;
    }

    for(auto& sk:m_socks){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"\nServer Name: "<<m_name
                                            <<" Listening socket: "<<sk->toString();
    }
    return true;
}
void TCP_Server::handleAccept(mySocket::ptr listen_sock){
    while(!m_isStop){
        // while保持监听，accept hook 实现 epoll IO多路复用
        mySocket::ptr client_sock=listen_sock->accept();
        if(client_sock){
            client_sock->setRecvTimeout(m_recvTimeout);
            m_worker->schedule(std::bind(&TCP_Server::handleClient,
                                    shared_from_this(),client_sock));   // 这里bind传一个智能指针而不是裸指针，防止内存泄漏
        }else{
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"TCP_Server handleAccept() ERROR: ";
        }
    }
}
bool TCP_Server::start(){
    if(!m_isStop){
        return true;
    }
    m_isStop=false;
    for(auto& listen_sock:m_socks){
        m_worker->schedule(std::bind(&TCP_Server::handleAccept,shared_from_this(),listen_sock));
    }
    return true;
}
void TCP_Server::stop(){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"TCP_Server stopping";
    m_isStop=true;

    // 关停所有 listens_sock
    for(auto& listen_sock:m_socks){
        m_worker->schedule([listen_sock](){
            listen_sock->CancelAll();
            listen_sock->close();
        });
    }
    m_socks.clear();
}
void TCP_Server::handleClient(mySocket::ptr client_sock){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"handleClient: "<<client_sock->toString();
    sleep(5);
}


}
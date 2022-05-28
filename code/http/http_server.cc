#include "http_server.h"

namespace myWeb{
namespace http{

Http_Server::Http_Server(bool keep_alive,IOManager* worker)
                    :TCP_Server(worker)
                    ,m_keepalive(keep_alive){}

void Http_Server::handleClient(mySocket::ptr client_sock){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"client_sock info: "<<client_sock->toString();
    HttpSession::ptr session(new HttpSession(client_sock,true));

    while(1){
        // 接收请求
        HttpRequest::ptr req=session->recvRequest();
        if(!req){
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"HTTP request recv failed, errno="<<errno<<"---"<<strerror(errno)
                                                <<" client: "<<client_sock->toString()<<"\nConnection: "<<m_keepalive;
        }
        // 打印 http_request
        std::string recv_buff=req->toString();
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"recv_len: "<<recv_buff.size()<<"\n"<<recv_buff;

        // 响应
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(),m_keepalive && req->isKeepAlive()));
        rsp->setHeaders("server",getName());
        std::string snd_buff="copy words: "+std::to_string(recv_buff.size());
        rsp->setBody(snd_buff);
        session->sendResponse(rsp);

        if(!m_keepalive || !req->isKeepAlive()){
            break;
        }
    }
    session->close();
}

}
}
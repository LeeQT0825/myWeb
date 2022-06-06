#include "http_server.h"

namespace myWeb{
namespace http{

Http_Server::Http_Server(bool keep_alive,IOManager* worker)
                    :TCP_Server(worker)
                    ,m_keepalive(keep_alive){
    m_type="http";
    m_Dispatch.reset(new ServletDispatch);
}

void Http_Server::handleClient(mySocket::ptr client_sock){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"client_sock info: "<<client_sock->toString();
    HttpSession::ptr session(new HttpSession(client_sock));

    while(1){
        // 接收请求
        HttpRequest::ptr req=session->recvRequest();
        if(!req){
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"HTTP request recv failed, errno="<<errno<<"---"<<strerror(errno)
                                                <<" client: "<<client_sock->toString()<<"\nConnection: "<<m_keepalive;
            break;
        }
        // 打印 http_request
        std::string recv_buff=req->toString();
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"recv_len: "<<recv_buff.size()<<"\n"<<recv_buff;

        // 响应（TODO 不添加任务的话会出现死循环的现象（可能是，这里的bug没完全解决））
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(),req->isKeepAlive() && m_keepalive));
        rsp->setHeaders("server",getName());
        m_Dispatch->handle(req,rsp,session);
        session->sendResponse(rsp);
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"response send";

        // m_worker->schedule([session,rsp](){
        //     session->sendResponse(rsp);
        //     INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"response send";
        // });
        
        if(!m_keepalive || !req->isKeepAlive()){
            // sleep(1);
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"session over";
            break;
        }
    }
    session->close();
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"session closed";
}

}
}
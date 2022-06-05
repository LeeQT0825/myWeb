#ifndef __MYWEB_HTTP_SERVER_H__
#define __MYWEB_HTTP_SERVER_H__

#include "http_session.h"
#include "servlet.h"
#include "../tcp_server.h"

namespace myWeb{
namespace http{

class Http_Server:public TCP_Server{
public:
    typedef std::shared_ptr<Http_Server> ptr;
    Http_Server(bool keep_alive=true,IOManager* worker=IOManager::getThis());

    //  设置 TCP_Server 名称
    void setName(const std::string& name){
        TCP_Server::setName(name);
        m_Dispatch->setDefaultServlet(std::make_shared<NotFoundServlet>(name));
    }
    // 设置servlet分发器
    void setServletDispatch(ServletDispatch::ptr dispatch){ m_Dispatch=dispatch; }
    // 获取servlet分发器
    ServletDispatch::ptr getServletDispatch() const { return m_Dispatch; }

protected:
    virtual void handleClient(mySocket::ptr client_sock) override ;

private:
    // 是否是长连接
    bool m_keepalive;
    ServletDispatch::ptr m_Dispatch;

};

}
}
#endif
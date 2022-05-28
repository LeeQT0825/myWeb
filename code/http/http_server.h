#ifndef __MYWEB_HTTP_SERVER_H__
#define __MYWEB_HTTP_SERVER_H__

#include "http_session.h"
#include "../tcp_server.h"

namespace myWeb{
namespace http{

class Http_Server:public TCP_Server{
public:
    typedef std::shared_ptr<Http_Server> ptr;
    Http_Server(bool keep_alive=true,IOManager* worker=IOManager::getThis());

    //  设置 TCP_Server 名称
    void setName(const std::string& name) { m_name=name; }

protected:
    virtual void handleClient(mySocket::ptr client_sock) override ;

private:
    // 是否是长连接
    bool m_keepalive;

};

}
}
#endif
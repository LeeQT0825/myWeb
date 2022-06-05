#ifndef __MYWEB_HTTP_SESSION_H__
#define __MYWEB_HTTP_SESSION_H__

#include "../streams/socket_stream.h"
#include "http.h"
#include "http_parser.h"

namespace myWeb{
namespace http{

/* HTTP server端应用层会话类 */
class HttpSession:public Socket_Stream{
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession(mySocket::ptr sock,bool auto_destruct=true);

    // 接收http请求
    HttpRequest::ptr recvRequest();
    // 回复http响应
    int sendResponse(HttpResponse::ptr rsp);

};

}
}
#endif
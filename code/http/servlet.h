#ifndef __MYWEB_HTTP_SERVLET_H__
#define __MYWEB_HTTP_SERVLET_H__

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "../mmutex.h"

namespace myWeb{
namespace http{

// http处理接口
class Servlet{
public:
    typedef std::shared_ptr<Servlet> ptr;

    Servlet(const std::string& name="")
        :m_name(name){}
    virtual ~Servlet(){}

    /* 处理接口
        返回值：0--成功，-1--失败 */
    virtual int32_t handle(HttpRequest::ptr req,HttpResponse::ptr rsp,HttpSession::ptr session)=0;
    // 获取该接口的名称
    const std::string& getName() const { return m_name; }

protected:
    std::string m_name;
};

// 函数式处理接口
class FunctionServlet: public Servlet{
public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t(HttpRequest::ptr,HttpResponse::ptr,HttpSession::ptr)> callback;

    FunctionServlet(callback cb);

    /* 处理接口
        返回值：0--成功，-1--失败 */
    int handle(HttpRequest::ptr req,HttpResponse::ptr rsp,HttpSession::ptr session) override;
private:
    // 处理回调
    callback m_cb;
};

// URI不存在的处理接口（默认命中）
class NotFoundServlet:public Servlet{
public:
    typedef std::shared_ptr<NotFoundServlet> ptr;
    NotFoundServlet(const std::string& server_name);

    /* 处理接口
        返回值：0--成功，-1--失败 */
    int handle(HttpRequest::ptr req,HttpResponse::ptr rsp,HttpSession::ptr session) override;

private:
    // 服务器名称
    std::string m_serverName;
    // 404 消息体内容
    std::string m_content;

};

// 根据URI处理接口分发
class ServletDispatch: public Servlet{
public:
    typedef std::shared_ptr<ServletDispatch> ptr;
    typedef RWmutex locktype;

    ServletDispatch();
    /* 处理接口
        返回值：0--成功，-1--失败 */
    int handle(HttpRequest::ptr req,HttpResponse::ptr rsp,HttpSession::ptr session) override;

    // 传指针方式添加精确处理程序
    void addServlet(const std::string& uri,Servlet::ptr slt);
    // 传回调方式添加精确处理程序
    void addServlet(const std::string& uri,FunctionServlet::callback cb);
    // 传指针方式添加模糊处理程序
    void addObsServlet(const std::string& obs_uri,Servlet::ptr slt);
    // 传回调方式添加模糊处理程序
    void addObsServlet(const std::string& obs_uri,FunctionServlet::callback cb);
    // 设置默认处理程序
    void setDefaultServlet(Servlet::ptr slt){ m_default=slt; }

    // 删除指定的精确处理程序
    void delServlet(const std::string& uri);
    // 删除指定的模糊处理程序
    void delObsServlet(const std::string& uri);

    // 获取指定的精确处理程序
    Servlet::ptr getServlet(const std::string& uri);
    // 获取指定的模糊处理程序
    Servlet::ptr getObsServlet(const std::string& uri);
    // 获取默认处理程序
    Servlet::ptr getDefaultServlet(){ return m_default; }
    // 自动匹配处理程序（精确、模糊、默认）
    Servlet::ptr getMatchedServlet(const std::string& uri);

private:
    locktype m_lock;
    // 根据URI精准命中处理接口（/myWeb/xxx）
    std::unordered_map<std::string,Servlet::ptr> m_preciseServlet;
    // 根据URI模糊匹配处理接口（/myweb/*）
    std::vector<std::pair<std::string,Servlet::ptr> > m_obsServlet;
    // 没有命中Servlet时匹配
    Servlet::ptr m_default;
};


}
}
#endif
#include "servlet.h"
#include <fnmatch.h>

namespace myWeb{
namespace http{

FunctionServlet::FunctionServlet(callback cb)
                        :Servlet("FunctionServlet")
                        ,m_cb(cb){}
int FunctionServlet::handle(HttpRequest::ptr req,HttpResponse::ptr rsp,HttpSession::ptr session){
    return m_cb(req,rsp,session);
}

NotFoundServlet::NotFoundServlet(const std::string& server_name)
                        :Servlet("NotFoundServlet")
                        ,m_serverName(server_name){
    m_content=  "<html>"
                "<head><title>404 Not Found</title></head>"
                "<body>"
                "<center><h1>404 Not Found</h1></center>"
                "<hr><center>"+m_serverName+"</hr></center>"
                "</html>";
}
int NotFoundServlet::handle(HttpRequest::ptr req,HttpResponse::ptr rsp,HttpSession::ptr session){
    rsp->setStatus(Http_Status::NOT_FOUND);
    rsp->setHeaders("server",m_serverName);
    rsp->setHeaders("content-type","text/html");
    rsp->setBody(m_content);
    return 0;
}

ServletDispatch::ServletDispatch()
                        :Servlet("ServletDispatch"){
    m_default.reset(new NotFoundServlet("myWeb/1.0.0"));
}
int ServletDispatch::handle(HttpRequest::ptr req,HttpResponse::ptr rsp,HttpSession::ptr session){
    auto stl=getMatchedServlet(req->getPath());
    if(stl){
        return stl->handle(req,rsp,session);
    }
    return -1;
} 
void ServletDispatch::addServlet(const std::string& uri,Servlet::ptr slt){
    locktype::write_lock wt_lck(m_lock);
    m_preciseServlet[uri]=slt;
}
void ServletDispatch::addServlet(const std::string& uri,FunctionServlet::callback cb){
    locktype::write_lock wt_lck(m_lock);
    m_preciseServlet[uri].reset(new FunctionServlet(cb));
}
void ServletDispatch::addObsServlet(const std::string& obs_uri,Servlet::ptr slt){
    locktype::write_lock wt_lck(m_lock);
    for(auto iter=m_obsServlet.begin();iter!=m_obsServlet.end();++iter){
        if(iter->first==obs_uri){
            iter->second=slt;
            return;
        }
    }
    m_obsServlet.push_back(std::make_pair(obs_uri,slt));
}
void ServletDispatch::addObsServlet(const std::string& obs_uri,FunctionServlet::callback cb){
    addObsServlet(obs_uri,std::make_shared<FunctionServlet>(cb));
}
void ServletDispatch::delServlet(const std::string& uri){
    locktype::write_lock wt_lck(m_lock);
    m_preciseServlet.erase(uri);
}
void ServletDispatch::delObsServlet(const std::string& uri){
    locktype::write_lock wt_lck(m_lock);
    for(auto iter=m_obsServlet.begin();iter!=m_obsServlet.end();++iter){
        if(iter->first==uri){
            m_obsServlet.erase(iter);
            return;
        }
    }
}
Servlet::ptr ServletDispatch::getServlet(const std::string& uri){
    locktype::read_lock rd_lck(m_lock);
    auto iter=m_preciseServlet.find(uri);
    return iter==m_preciseServlet.end() ? nullptr : iter->second;
}
Servlet::ptr ServletDispatch::getObsServlet(const std::string& uri){
    locktype::read_lock rd_lck(m_lock);
    for(auto iter=m_obsServlet.begin();iter!=m_obsServlet.end();++iter){
        if(iter->first==uri){
            return iter->second;
        }
    }
    return nullptr;
}
Servlet::ptr ServletDispatch::getMatchedServlet(const std::string& uri){
    locktype::read_lock rd_lck(m_lock);

    auto iter=m_preciseServlet.find(uri);
    if(iter!=m_preciseServlet.end()){
        return iter->second;
    }

    for(auto it=m_obsServlet.begin();it!=m_obsServlet.end();++it){
        if(!fnmatch(it->first.c_str(),uri.c_str(),0)){
            return it->second;
        }
    }

    return m_default;
}

}
}
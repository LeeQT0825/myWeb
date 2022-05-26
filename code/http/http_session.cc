#include "http_session.h"

namespace myWeb{
namespace http{

HttpSession::HttpSession(mySocket::ptr sock,bool auto_destruct)
                    :Socket_Stream(sock,auto_destruct){}

HttpRequest::ptr HttpSession::recvRequest(){
    HttpRequestParser::ptr parser(new HttpRequestParser());
    uint64_t buff_size=HttpRequestParser::GetReq_BufferSize();
    // 用智能指针封装接收缓存
    std::shared_ptr<char> shr_buff(new char[buff_size],[](char* pointer){
        delete[] pointer;
    });
    char* recv_buff=shr_buff.get();
    memset(recv_buff,'\0',buff_size);

    size_t offset=0;

    while(true){
        int len=read(recv_buff+offset,buff_size-offset);
        if(len<=0){
            close();
            return nullptr;
        }
        size_t ps_len=parser->execute(recv_buff,len+offset);
        if(parser->isError()){
            close();
            return nullptr;
        }
        
        // 更新offset
        offset=len+offset-ps_len;
        if(offset==buff_size){
            close();
            return nullptr;
        }

        if(parser->isFinished()>0){
            break;
            // 此时offset为recv_buff剩余数据，均为body
        }
    }

    size_t conten_len=parser->getContentLength();
    if(conten_len>0){
        std::string body;
        body.resize(conten_len);
        
        if(conten_len>offset){
            memcpy(&body[0],recv_buff,offset);
            conten_len-=offset;
            int ret=read_Fix(&body[offset],conten_len);
            if(ret<=0){
                close();
                return nullptr;
            }
        }else{
            memcpy(&body[0],recv_buff,conten_len);
        }

        parser->getReqData()->setBody(body);
    }

    parser->getReqData()->init();
    return parser->getReqData();
}
int HttpSession::sendResponse(HttpResponse::ptr rsp){
    std::stringstream ss;
    rsp->dump(ss);
    std::string rsp_buff=ss.str();

    return write_Fix(&rsp_buff[0],rsp_buff.size());
}

}
}
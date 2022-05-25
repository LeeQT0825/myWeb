#include "../code/myweb.h"

class Echo_Server:public myWeb::TCP_Server{
public:
    Echo_Server(){}

    void handleClient(myWeb::mySocket::ptr client_sock);

private:

};

static size_t s_write_buffer_size=1024;

void Echo_Server::handleClient(myWeb::mySocket::ptr client_sock){
    // 功能：接收信息，通过序列化
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"client_sock info: "<<client_sock->toString();

    myWeb::ByteArray::ptr barray(new myWeb::ByteArray);

    while(1){
        barray->clear();
        // 设置缓存（集中写）
        std::vector<iovec> iovs;
        barray->getWriteBuffs(iovs,s_write_buffer_size);

        int ret=client_sock->recv(&iovs[0],iovs.size());
        if(ret==0){
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"Client closed"<<client_sock->toString();
            break;
        }else if(ret<0){
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"Client error: "<<errno<<"---"<<strerror(errno);
            break;
        }

        // 设置可读范围
        size_t pre_pos=barray->getPosition();
        barray->setPosition(ret+pre_pos);

        // 设置读起点
        barray->setPosition(pre_pos);
        size_t recv_len=barray->getRestRdSize();
        MYWEB_ASSERT(ret==(int)recv_len);

        // 检查是否能接收全部信息
        std::string dump_str=barray->toString();
        int dump_len=dump_str.size();
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"origin msg: \n"<<dump_str;

        // 测试 http parser
        myWeb::http::HttpRequestParser::ptr http_req_parser(new myWeb::http::HttpRequestParser());

        ret=http_req_parser->execute(&dump_str[0],dump_len);
        dump_str.resize(dump_len-ret);

        // 输出 http parser 信息
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"http_parser info: \n execute ret= "<<ret<<"/"<<recv_len
                                            <<"\n error= "<<http_req_parser->getError()
                                            <<"\n is finish= "<<http_req_parser->isFinished()
                                            <<"\n Content Length= "<<http_req_parser->getContentLength()<<"\n";
        
        // 输出 http_request
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<client_sock->getDstAddr()->toString()<<": ";
        http_req_parser->getReqData()->dump(std::cout);      // 输出消息体
        std::cout.flush();

        // 输出 http_body
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<dump_str.size()<<" body: \n"<<dump_str;

        // 回应给客户端接收信息
        std::string echo_buff="copy: "+std::to_string(recv_len)+" Bytes";
        client_sock->send(&echo_buff[0],echo_buff.size());
    }
}

void run(){
    std::vector<myWeb::Address::ptr> addrs;
    myWeb::Address::Lookup(addrs,"0.0.0.0:12345");
    if(addrs.empty())   return;
    myWeb::Address::ptr addr=addrs[0];

    Echo_Server::ptr echo_server(new Echo_Server);
    std::vector<myWeb::Address::ptr> f_addrs;
    if(echo_server->bind_listen(addr)){
        sleep(2);
        echo_server->start();
    }
}


int main(int argc,char** argv){
    myWeb::IOManager::ptr iom(new myWeb::IOManager(3));
    iom->schedule(run);

    return 0;
}
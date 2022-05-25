#include "../code/myweb.h"

const char req_buff[]=  "GET /favicon.ico HTTP/1.1\r\n"
                        "Host: localhost:12345\r\n"
                        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:96.0) Gecko/20100101 Firefox/96.0\r\n"
                        "Accept: image/avif,image/webp,*/*\r\n"
                        "Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "Connection: keep-alive\r\n"
                        "Referer: http://localhost:12345/\r\n"
                        "Sec-Fetch-Dest: image\r\n"
                        "Sec-Fetch-Mode: no-cors\r\n"
                        "Sec-Fetch-Site: same-origin\r\n"
                        "Cache-Control: max-age=0\r\n\r\n"
                        "aaabbbcccddd";

void run(){
    myWeb::IP_Address::ptr addr=myWeb::IP_Address::Create_addr("127.0.0.1",12345);
    myWeb::mySocket::ptr client_sock=myWeb::mySocket::CreateTCPsocket4();
    myWeb::IOManager* iom=myWeb::IOManager::getThis();

    std::string snd_buff=req_buff;
    std::string rcv_buff;
    rcv_buff.resize(64);

    bool ret=client_sock->connect(addr,2*1000);
    assert(ret==true);

    iom->addCondTimer(2*1000,[client_sock,snd_buff](){
        int ret=client_sock->send(&snd_buff[0],snd_buff.size());
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"send: "<<ret;
    },std::weak_ptr<myWeb::mySocket>(client_sock),true);

    iom->addTimer(10*1000,[client_sock](){
        client_sock->close();
    });

    while(client_sock->isConnected()){
        memset(&rcv_buff[0],0,sizeof(rcv_buff));
        
        client_sock->recv(&rcv_buff[0],sizeof(rcv_buff));
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<rcv_buff;
    }
}

int main(int argc,char** argv){
    myWeb::IOManager::ptr iom(new myWeb::IOManager(3));
    iom->schedule(run);

    return 0;
}
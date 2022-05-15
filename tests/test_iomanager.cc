#include "../code/myweb.h"
#include "../code/iomanager.h"
#include "../code/hook.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>     // 字节序转换
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <functional>

myWeb::Timer::ptr timer;
void test_rd_wr(int fd);

void test_listen(const char* ip,int port){
    // 创建socket地址
    sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    inet_pton(AF_INET,ip,&addr.sin_addr);

    // 创建 listenfd ，非阻塞
    int listenfd=socket(PF_INET,SOCK_STREAM,0);
    MYWEB_ASSERT(listenfd>=0);
    // 复用socket地址
    int reused=1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reused,sizeof(reused));

    int ret=bind(listenfd,(const sockaddr*)&addr,sizeof(addr));
    MYWEB_ASSERT(ret==0);

    ret=listen(listenfd,5);
    MYWEB_ASSERT(ret!=-1);
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"listenfd: "<<listenfd;

    // 链接
    sockaddr_in connaddr;
    socklen_t connaddr_len=sizeof(connaddr);
    int connfd=accept(listenfd,(sockaddr*)&connaddr,&connaddr_len);

    if(connfd>=0){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"accept success: "<<connfd;
        sockaddr_in conn_addr;
        socklen_t conn_len=sizeof(conn_addr);
        ret=getpeername(connfd,(sockaddr*)&conn_addr,&conn_len);
        if(!ret){
            char str_ip[INET_ADDRSTRLEN];
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<inet_ntop(AF_INET,&conn_addr.sin_addr,str_ip,conn_len)
                                                <<":"<<ntohs(conn_addr.sin_port);
        }
        
        // myWeb::IOManager::getThis()->schedule(std::bind(test_rd_wr,connfd));
        sleep(15);
        // 关闭
        ret=shutdown(connfd,SHUT_RDWR);
        if(ret==0){
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"accept shutdown success: "<<connfd;
        }else{
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"accept shutdown failed: "<<connfd;
        }

    }else{
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"accept failed: "<<connfd;
    }

}

void test_connect(const char* ip,int port){
    // 创建socket地址
    sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    inet_pton(AF_INET,ip,&addr.sin_addr);

    int sockfd=socket(PF_INET,SOCK_STREAM,0);
    MYWEB_ASSERT(sockfd>=0);
    // 复用socket地址
    int reused=1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reused,sizeof(reused));

    int ret=connect(sockfd,(const sockaddr*)&addr,sizeof(addr));
    if(ret==0){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"connect success: "<<sockfd;
        
        myWeb::IOManager::getThis()->schedule(std::bind(test_rd_wr,sockfd));
        sleep(10);

        // 关闭
        ret=shutdown(sockfd,SHUT_RDWR);
        if(ret==0){
            INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"connect shutdown success: "<<sockfd;
        }else{
            INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"connect shutdown failed: "<<sockfd;
        }

    }else{
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"connect failed: "<<sockfd;
    }

}

void test_rd_wr(int fd){
    const char buf[]="GET / HTTP/1.0\r\n\r\n";
    int ret=send(fd,buf,sizeof(buf),0);
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"send ret= "<<ret<<" errno= "<<errno;
    
    std::string recv_buf;
    recv_buf.resize(4096);
    ret=recv(fd,&recv_buf[0],sizeof(recv_buf),0);
    
    if(ret<0){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"recv ret= "<<ret<<" errno= "<<errno;
        return;
    }   

    recv_buf.resize(ret);
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<recv_buf;
}

void test_Timer(myWeb::IOManager::ptr& iomanager){  
    timer=iomanager->addTimer(500,[](){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"hello timer_1: "<<myWeb::GetCurrentMS();
        static int a=0;
 
        if(++a==5){
            // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"timer cancel";
            timer->cancelTimer();
            // timer->resetTimer(2000,true);
        }
        sleep(10);
    },true);
}

void set_nonBlock(int fd){
    int old_op=fcntl(fd,F_GETFL);
    int new_op=old_op | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_op);
}

int main(int argc,char** argv){
    if(argc<=2){
        std::cout<<"usage: "<<basename(argv[0])<<" IP_addr Port_num"<<std::endl;
        return 1;
    }
    const char* ip=argv[1];
    int port=atoi(argv[2]);

    LOADYAML;

    myWeb::IOManager::ptr iomanager(new myWeb::IOManager(5));
    
    // test_Timer(iomanager);

    iomanager->schedule(std::bind(test_listen,ip,port));

    // iomanager->schedule(std::bind(test_connect,"180.101.49.12",80));

    // sleep(10);
    return 0;
}
#include "../code/myweb.h"
#include "../code/iomanager.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>     // 字节序转换
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

myWeb::Timer::ptr timer;

void test_listen(int listenfd){
    INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"get in test_listen";

    sockaddr_in connaddr;
    socklen_t connaddr_len=sizeof(connaddr);
    // 链接
    int connfd=accept(listenfd,(sockaddr*)&connaddr,&connaddr_len);
    if(connfd>=0){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"connect success: "<<connfd;
    }else{
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"connect failed: "<<connfd;
    }

    sleep(5);

    // 关闭
    int ret=shutdown(connfd,SHUT_RDWR);
    if(ret==0){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"shutdown success: "<<connfd;
    }else{
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"shutdown failed: "<<connfd;
    }
    // 重新加入链接监听
    // myWeb::IOManager::getThis()->addEvent(listenfd,myWeb::IOManager::Event::READ,
    //                 std::bind(test_listen,listenfd));
}

void test_Timer(myWeb::IOManager::ptr& iomanager){  
    timer=iomanager->addTimer(500,[](){
        INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"hello timer_1: "<<myWeb::GetCurrentMS();
        static int a=0;
        if(++a==5){
            // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"timer cancel";
            // timer->cancelTimer();
            timer->resetTimer(2000,true);
        }
    },true);
}

void func(){
    
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
    
    test_Timer(iomanager);

    // 创建socket地址
    sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    inet_pton(AF_INET,ip,&addr.sin_addr);

    // // 创建 listenfd ，非阻塞
    // int listenfd=socket(PF_INET,SOCK_STREAM,0);
    // MYWEB_ASSERT(listenfd>=0);
    // set_nonBlock(listenfd);
    // // 复用socket地址
    // int reused=1;
    // setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reused,sizeof(reused));

    // int ret=bind(listenfd,(const sockaddr*)&addr,sizeof(addr));
    // MYWEB_ASSERT(ret==0);

    // ret=listen(listenfd,5);
    // MYWEB_ASSERT(ret!=-1);
    // INLOG_INFO(MYWEB_NAMED_LOG("system"))<<"listenfd: "<<listenfd;

    // // 注册listenfd的读事件
    // iomanager->addEvent(listenfd,myWeb::IOManager::Event::READ,std::bind(test_listen,listenfd));

    // sleep(30);
    return 0;
}
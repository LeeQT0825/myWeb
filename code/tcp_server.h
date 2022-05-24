#ifndef __MYWEB_TCP_SERVER_H__
#define __MYWEB_TCP_SERVER_H__

#include <memory>
#include <functional>
#include <vector>
#include "iomanager.h"
#include "socket.h"
#include "config.h"

namespace myWeb{

class TCP_Server : public std::enable_shared_from_this<TCP_Server>{
public:
    typedef std::shared_ptr<TCP_Server> ptr;

    TCP_Server(IOManager* worker=IOManager::getThis());
    virtual ~TCP_Server();

    // 绑定并监听地址
    virtual bool bind_listen(Address::ptr addr);
    /* 绑定并监听
        addrs: 待绑定的地址
        f_addrs: 绑定失败的地址 */
    virtual bool bind_listen(const std::vector<Address::ptr>& addrs,std::vector<Address::ptr>& f_addrs);
    // 执行 handleAccept ，创建连接
    virtual bool start();
    // 关闭 server
    virtual void stop();

    // 获取信息接收超时时间
    uint64_t getRecvTimeOut() const { return m_recvTimeout; }
    //  获取 TCP_Server 名称
    std::string getName() const { return m_name; }
    // 设置信息接收超时时间
    void setRecvTimeOut(uint64_t val) { m_recvTimeout=val; }
    //  设置 TCP_Server 名称
    void setName(const std::string& name) { m_name=name; }
    // 判断是否停止
    bool isStop() const { return m_isStop; }

protected:
    // 处理新连接的 mySocket 类
    virtual void handleClient(mySocket::ptr client_sock);
    // 接收连接
    virtual void handleAccept(mySocket::ptr listen_sock);

private:
    TCP_Server(const TCP_Server& ) = delete;
    TCP_Server& operator=(const TCP_Server&) = delete;

protected:
    // 存放监听socket，支持多地址监听，所以用vector
    std::vector<mySocket::ptr> m_socks;
    // 任务调度器
    IOManager* m_worker;
    // 防止恶意连接，长时间无对话就关掉
    uint64_t m_recvTimeout;
    // 为做日志输出
    std::string m_name;
    // 是否停止
    bool m_isStop;

};


}
#endif
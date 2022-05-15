#ifndef __MYWEB_HOOK_H__
#define __MYWEB_HOOK_H__

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <sys/uio.h>

namespace myWeb{

// 判断是否可以使用 hook 后的函数
bool is_hook_enable();
// 控制使能hook
void set_hook_enable(bool flag);

}


// 用C规则编译指定的代码
extern "C"{

// 类型简化————函数指针
typedef unsigned int (*sleep_func)(unsigned int seconds);
typedef int (*usleep_func)(unsigned int usec);
typedef int (*nanosleep_func)(const struct timespec *req, struct timespec *rem);
typedef int (*socket_func)(int domain, int type, int protocol);
typedef int (*connect_func)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
typedef int (*accept_func)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
typedef ssize_t (*read_func)(int fd, void *buf, size_t count);
typedef ssize_t (*write_func)(int fd, const void *buf, size_t count);
typedef ssize_t (*readv_func)(int fd, const struct iovec *iov, int iovcnt);
typedef ssize_t (*writev_func)(int fd, const struct iovec *iov, int iovcnt);
typedef ssize_t (*recv_func)(int sockfd, void *buf, size_t len, int flags);
typedef ssize_t (*send_func)(int sockfd, const void *buf, size_t len, int flags);
typedef ssize_t (*recvfrom_func)(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
typedef ssize_t (*sendto_func)(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
typedef ssize_t (*recvmsg_func)(int sockfd, struct msghdr *msg, int flags);
typedef ssize_t (*sendmsg_func)(int sockfd, const struct msghdr *msg, int flags);
typedef int (*close_func)(int fd);
typedef int (*fcntl_func)(int fd, int cmd, ...);
typedef int (*ioctl_func)(int fd, unsigned long request, ...);
typedef int (*getsockopt_func)(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
typedef int (*setsockopt_func)(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

// 类型声明
extern sleep_func sleep_f;
extern usleep_func usleep_f;
extern nanosleep_func nanosleep_f;
extern socket_func socket_f;
extern connect_func connect_f;
extern accept_func accept_f;
extern read_func read_f;
extern write_func write_f;
extern readv_func readv_f;
extern writev_func writev_f;
extern recv_func recv_f;
extern send_func send_f;
extern recvfrom_func recvfrom_f;
extern sendto_func sendto_f;
extern recvmsg_func recvmsg_f;
extern sendmsg_func sendmsg_f;
extern close_func close_f;
extern fcntl_func fcntl_f;
extern ioctl_func ioctl_f;
extern getsockopt_func getsockopt_f;
extern setsockopt_func setsockopt_f;

extern int connect_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen,uint64_t timeout_ms);
}


#endif
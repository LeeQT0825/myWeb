#ifndef __MYWEB_HOOK_H__
#define __MYWEB_HOOK_H__

#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
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

// 类型声明
extern sleep_func sleep_f;
extern usleep_func usleep_f;

// 函数声明
unsigned int sleep(unsigned int seconds);
int usleep(useconds_t usec);


}


#endif
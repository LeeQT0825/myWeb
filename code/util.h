// 辅助函数库

#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>

#ifndef __MYWEB_UTIL_H__
#define __MYWEB_UTIL_H__
namespace myWeb{

// 获取当前的系统线程号
pid_t GetThreadID();
// 获取协程ID
int32_t GetFiberID();

}
#endif
#ifndef __MACRO_H__
#define __MACRO_H__

#include <assert.h>
#include "log.h"
#include "util.h"

#define LOADYAML myWeb::Config::LoadFromYaml("/home/lee/projects/VScode/myProject/myconfig.yml");

// 封装断言
// TODO 为什么在主函数中定义system log的appenders，却无法真正执行呢? 执行的依然是默认appenders。
#define MYWEB_ASSERT(x) \
    if(!(x)){\
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"ASSERTION: " \
                <<#x<<"\n backtrace:" \
                <<myWeb::BacktraceToString(5); \
        assert(x); \
    }
#undef x

#endif 
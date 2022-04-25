#ifndef __MACRO_H__
#define __MACRO_H__

#include <assert.h>
#include "log.h"
#include "util.h"

#define LOADYAML myWeb::Config::LoadFromYaml("/home/lee/projects/VScode/myProject/myconfig.yml");

// 封装断言
#define MYWEB_ASSERT(x) \
    if(!(x)){\
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"ASSERTION: " \
                <<#x<<"\n backtrace:" \
                <<myWeb::BacktraceToString(10); \
        assert(x); \
    }
#undef x
#define MYWEB_ASSERT_2(x,y) \
    if(!(x)){\
        INLOG_ERROR(MYWEB_NAMED_LOG("system"))<<"ASSERTION: " \
                <<y<<"\n backtrace:" \
                <<myWeb::BacktraceToString(10); \
        assert(x); \
    }
#undef x
#undef y

#endif 
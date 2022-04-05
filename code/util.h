// 辅助函数库

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#ifndef __MYWEB_UTIL_H__
#define __MYWEB_UTIL_H__
namespace myWeb{

// 获取当前的系统线程号
pid_t GetThreadID();
// 获取协程ID
int32_t GetFiberID();

// 文件相关
class FileUtils{
public: 
    // 创建路径
    static bool Mkdir(std::string& dir);
    // 从绝对文件名中获取路径名
    static std::string DirName(const std::string& filename);
    // 从绝对文件名中获取文件名
    static std::string BaseName(const std::string& filename);
    static bool OpenForWrite(std::ofstream& ofs,const std::string& filename,std::ios_base::openmode mode);
    static bool OpenForRead(std::ifstream& ifs,const std::string& filename,std::ios_base::openmode mode);
};

}
#endif
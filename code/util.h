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
#include <byteswap.h>

#ifndef __MYWEB_UTIL_H__
#define __MYWEB_UTIL_H__
// 字节序
#define MYWEB_LITTLE_ENDIAN 1
#define MYWEB_BIG_ENDIAN 2
namespace myWeb{

// 获取当前的系统线程号
pid_t GetThreadID();
// 获取协程ID
uint64_t GetFiberID();



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

std::string BacktraceToString(int size,int skip=1,const std::string& prefix="\n---");



// 时间相关ms
// 毫秒
uint64_t GetCurrentMS();
// 微秒
uint64_t GetCurrentUS();



// 字节序转化

template<typename T>
typename std::enable_if<sizeof(T)==sizeof(uint16_t),T>::type ByteSwap(T val){
    return (T)bswap_16(val);
}

template<typename T>
typename std::enable_if<sizeof(T)==sizeof(uint32_t),T>::type ByteSwap(T val){
    return (T)bswap_32(val);
}

template<typename T>
typename std::enable_if<sizeof(T)==sizeof(uint64_t),T>::type ByteSwap(T val){
    return (T)bswap_64(val);
}


// 判断当前系统的字节序
#if BYTE_ORDER == BIG_ENDIAN
#define MYWEB_ORDER MYWEB_BIG_ENDIAN
#else
#define MYWEB_ORDER MYWEB_LITTLE_ENDIAN
#endif

}
#endif
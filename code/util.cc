#include "util.h"
#include <string.h>
#include <execinfo.h>       // backtrace() 系列函数
#include <sstream>
#include <cxxabi.h>
namespace myWeb{

pid_t GetThreadID(){
    // 1. gettid() ---系统调用(开销较大)，可以用宏"__NR_gettid"替代，获取的是内核中真实的线程ID
    // （linux内核其实没有线程的概念，所以内核线程就是“内核进程”，所以返回的是pid_t，当用户进程中只有一个线程的时候返回的是该进程号
    // 2. pthread_self() 获取的是POSIX描述的线程ID（是相对于进程的线程控制块的首地址，用户线程）
    // 3. std::thread::get_id() 获取的是用户线程ID
    return syscall(__NR_gettid);       
}

int32_t GetFiberID(){
    return 0;
}

// 封装lstat()系统调用，获取软链接（符号链接）本身的属性，存储到st中。0表示成功，-1表示错误
static int __lstat(const char* file,struct stat* st=nullptr){
    struct stat lst;
    int ret=lstat(file,&lst);
    if(ret>=0){
        *st=lst;
    }
    return ret;
}
// 封装mkdir()系统调用，创建路径。0表示成功，-1表示错误
static int __mkdir(const char* dir){
    if(access(dir,F_OK)==0){
        return 0;
    }
    return mkdir(dir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
// 创建路径
bool FileUtils::Mkdir(std::string& dir){
    if(__lstat(dir.c_str())==0) {return true;}
    char* path=strdup(dir.c_str());          // 字符串拷贝，与free() 搭配
    char* pos=strchr(path+1,'/');            // 为找到返回nullptr,默认dir首字符是'/'
    for(;pos;*pos='/',pos=strchr(pos+1,'/')){
        *pos='\0';
        if(__mkdir(path)!=0)    break;       //path字符串是需要累加的
    }
    if(pos!=nullptr || __mkdir(dir.c_str())!=0){
        free(path);
        return false;
    }    
    free(path);
    return true;
}
std::string FileUtils::DirName(const std::string& filename){
    if(filename.empty()){
        return ".";     // 当前文件夹
    }
    auto pos=filename.rfind('/');       //Index of last occurrence. If found, returns the index where it was found. If not found, returns npos.
    if(pos==0){
        return "/";     // 一级文件夹内
    }else if(pos==std::string::npos){
        return ".";
    }else{
        return filename.substr(0,pos);
    }
}
std::string FileUtils::BaseName(const std::string& filename){
    if(filename.empty()){
        return filename;
    }
    auto pos=filename.rfind('/');
    if(pos==std::string::npos){
        return filename;
    }else{
        return filename.substr(pos+1);
    }
}
bool FileUtils::OpenForWrite(std::ofstream& ofs,const std::string& filename,std::ios_base::openmode mode){
    ofs.open(filename.c_str(),mode);
    if(!ofs.is_open()){
        std::string dir=DirName(filename);
        Mkdir(dir);
        ofs.open(filename.c_str(),mode);
    }
    return ofs.is_open();
}
bool FileUtils::OpenForRead(std::ifstream& ifs,const std::string& filename,std::ios_base::openmode mode){
    ifs.open(filename.c_str(),mode);
    return ifs.is_open();
}

// 调试相关
// demangle 字符转换
std::string demangle(const char* s){
    size_t size = 0;
    int status = 0;
    std::string temp(256,0);
    char *demangled = NULL;
    // C++样式
    if (sscanf(s, "%*[^(]%*[^_]%255[^)+]", &temp[0]) == 1) {
        if ((demangled = abi::__cxa_demangle(&temp[0], NULL, &size,
                        &status)) != NULL) {
            std::string result(demangled);
            free(demangled);
            return result;
        }
    }
    // C样式
    if (sscanf(s, "%255s", &temp[0]) == 1) {
        return temp;
    }
    // 原样式
    return s;
}
// 返回栈 bt--存储栈内容的数组  size--最大栈层数 skip--跳过几层栈（比如跳过自己）
void Backtrace(std::vector<std::string>& bt,int size,int skip=1){
    void** array=(void**)malloc(sizeof(void*)*size);
    int s=::backtrace(array,size);       // 实际返回栈层数

    char** strings=::backtrace_symbols(array,s);
    if(strings==nullptr){
        std::cout<<"backtrace_symbols error"<<std::endl;
        free(array);
        free(strings);
        return;
    }
    for(int i=skip;i<s;++i){
        bt.push_back(demangle(strings[i]));
    }
    free(array);
    free(strings);
}
// 返回栈接口
std::string BacktraceToString(int size,int skip,const std::string& prefix){
    std::vector<std::string> bt;
    Backtrace(bt,size,skip);
    std::stringstream ss;
    for(auto& i:bt){
        ss<<prefix<<i;
    }
    ss<<std::endl;
    return ss.str();
}

}
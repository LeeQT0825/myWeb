#include "stream.h"

namespace myWeb{

int Stream::read_Fix(void* buffer,size_t length){
    size_t offset=0;
    int left=length;

    while(length>0){
        int len=read((char*)buffer+offset,left);
        if(len<0){
            return len;
        }
        offset+=len;
        left-=len;
    }
    return length;
}
int Stream::read_Fix(ByteArray::ptr ba,size_t length){
    int left=length;

    while(left){
        int len=read(ba,left);
        if(len<0){
            return len;
        }
        left-=len;
    }
    return length;
}
int Stream::write_Fix(const void* buffer,size_t length){
    size_t offset=0;
    int left=length;

    while(left>0){
        int len=write((const char*)buffer+offset,left);
        if(len<0){
            return len;
        }
        offset+=len;
        left-=len;
    }
    return length;
}
int Stream::write_Fix(ByteArray::ptr ba,size_t length){
    int left=length;

    while(left){
        int len=write(ba,left);
        if(len<0){
            return len;
        }
        left-=len;
    }
    return length;
}
    
}
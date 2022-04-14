#include "mmutex.h"

namespace myWeb{

Semaphore::Semaphore(uint32_t count){
    if(sem_init(&m_semaphore,0,count)){
        INLOG_ERROR(MYWEB_ROOT_LOG)<<"semaphore init error";
        throw std::logic_error("semaphore init error");
    }
}

Semaphore::~Semaphore(){
    if(sem_destroy(&m_semaphore)){
        INLOG_ERROR(MYWEB_ROOT_LOG)<<"semaphore destroy error";
    }
}

void Semaphore::wait(){
    // TODO：这里用sem_wait()阻塞会性能损失
    if(sem_wait(&m_semaphore)){ 
        INLOG_ERROR(MYWEB_ROOT_LOG)<<"semaphore wait error";
        throw std::logic_error("semaphore wait error");        
    }
}

void Semaphore::post(){
    if(sem_post(&m_semaphore)){
        INLOG_ERROR(MYWEB_ROOT_LOG)<<"semaphore post error";
        throw std::logic_error("semaphore post error");
    }
}

}
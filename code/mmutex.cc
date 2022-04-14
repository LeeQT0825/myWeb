#include "mmutex.h"

namespace myWeb{

Semaphore::Semaphore(uint32_t count){
    if(sem_init(&m_semaphore,0,count)){
        throw std::logic_error("semaphore init error");
    }
}

Semaphore::~Semaphore(){    
    sem_destroy(&m_semaphore);
}

void Semaphore::wait(){
    // TODO：这里用sem_wait()阻塞会性能损失
    if(sem_wait(&m_semaphore)){ 
        throw std::logic_error("semaphore wait error");        
    }
}

void Semaphore::post(){
    if(sem_post(&m_semaphore)){
        throw std::logic_error("semaphore post error");
    }
}

}
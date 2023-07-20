#include "AsyncLogging.h"
#include "LogFile.h"

#include <chrono>

#include <iostream>

using namespace muduozdh;

AsyncLogging::AsyncLogging(std::string logFileName, int flushInterval)
    : flushInterval_(flushInterval), running_(false), basename_(logFileName), thread_(std::bind(&AsyncLogging::threadFunc,this),"Logging"),
      mutex_(), cond_(), currentBuffer_(new Buffer), nextBuffer_(new Buffer), buffers_() , startMutex_(){

    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
    

}

void AsyncLogging::append(const char* logline, int len){

    std::unique_lock<std::mutex> lock(mutex_);
    if(currentBuffer_->avail() > len){
        currentBuffer_->append(logline, len);

    }
    else{
        buffers_.push_back(currentBuffer_);
        currentBuffer_.reset();
        if(nextBuffer_)
            currentBuffer_ = std::move(nextBuffer_);
        else
            currentBuffer_.reset(new Buffer);
        currentBuffer_->append(logline, len);
        cond_.notify_one();
    }
}

void AsyncLogging::threadFunc(){

    
    startMutex_.unlock();

    LogFile output(basename_, 1024*1024*10);

    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    std::cout<<running_<<std::endl;

    while (running_){

        {
            std::unique_lock<std::mutex> lock(mutex_);
         
            if(buffers_.empty()){
                cond_.wait_for(lock,std::chrono::seconds(flushInterval_));
            }
            buffers_.push_back(currentBuffer_);
            currentBuffer_.reset();

            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_){
                nextBuffer_ = std::move(newBuffer2);
            }

        }


        for (size_t i = 0;i < buffersToWrite.size(); ++i){
            output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
        }

        if(buffersToWrite.size() > 2){
            buffersToWrite.resize(2);
        }

        if(!newBuffer1){
            newBuffer1 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if(!newBuffer2){
            newBuffer2 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }

    output.flush();

}



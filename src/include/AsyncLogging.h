#pragma once

#include "noncopyable.h"
#include "LogStream.h"
#include "Thread.h"

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace muduozdh{

class AsyncLogging : noncopyable{

public:

    AsyncLogging(const std::string basename, int flushInterval = 2);
    ~AsyncLogging(){
        if(running_) stop();
    }

    void append(const char* logline, int len);

    void start(){

        running_ = true;
        thread_ .start();
        startMutex_.lock();

    }

    void stop(){
        
        running_ = false;
        cond_.notify_one();
        // thread_.detach();
    }

private:

    using Buffer = FixedBuffer<kLargeBuffer> ;
    using BufferVector = std::vector<std::shared_ptr<Buffer>>;
    using BufferPtr = std::shared_ptr<Buffer>;

    void threadFunc();
    
    const int flushInterval_;
    bool running_;
    std::string basename_;
    Thread thread_;
    

    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;

    std::mutex mutex_;
    std::condition_variable cond_;

    std::mutex startMutex_;


};

}
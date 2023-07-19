#include "Logging.h"
#include "AsyncLogging.h"

#include <memory>

#include <iostream>

using namespace muduozdh;

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static std::unique_ptr<AsyncLogging> AsyncLogger_;

std::string Logger::logFileName_ = "./WebServer.log";


void once_init(){

    AsyncLogger_.reset(new AsyncLogging(Logger::getLogFileName()));
    AsyncLogger_->start();
    
}

void output(const char* msg, int len){
    pthread_once(&once_control_, once_init);
    AsyncLogger_->append(msg, len);
}

Logger::Impl::Impl(const char* filename, int line)
    : stream_(), line_(line), basename_(filename) {

        formatTime();
}

void Logger::Impl::formatTime(){

    std::string currentTime = Timestamp::now().toString();
    stream_<<currentTime<<" ";

}

Logger::Logger(const char *filename,int line)
    :impl_(filename, line) {}

Logger::~Logger(){

    impl_.stream_<< "  --  " << impl_.basename_ << ':' << impl_.line_ << '\n';

    std::cout<<impl_.stream_.buffer().data()<<std::endl;
    
    const LogStream::Buffer& buf(stream().buffer());
    output(buf.data(), buf.length());
    
}
#include "LogFile.h"

using namespace muduozdh;

LogFile::LogFile(const std::string& basename, int flushEveryN)
    :file_(new AppendFile(basename)), basename_(basename), flushEveryN_(flushEveryN), count_(0) { }

LogFile::~LogFile() {}

void LogFile::append(const char* logline, int len){

    // MutexLockGuard lock(*mutex);
    std::unique_lock<std::mutex> lock(mutex_);
    append_unlocked(logline, len);
}

void LogFile::flush(){

    std::unique_lock<std::mutex> lock(mutex_);
    file_->flush();
}

void LogFile::append_unlocked(const char* logline, int len){

    file_->append(logline, len);
    ++count_;
    if (count_ >= flushEveryN_){
        count_ = 0;
        file_->flush();
    }

}
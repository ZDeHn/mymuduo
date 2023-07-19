#pragma once

#include "noncopyable.h"
#include "FileUtil.h"

#include <string>
#include <memory>
#include <mutex>

namespace muduozdh{

class LogFile : noncopyable{

public:

    LogFile(const std::string& basename, int flushEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool roolFile();

private:

    void append_unlocked(const char* logline, int len);

    const std::string basename_;
    const int flushEveryN_;

    int count_;
    std::mutex mutex_;
    std::unique_ptr<AppendFile> file_;


};


}
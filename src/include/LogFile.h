#pragma once

#include "noncopyable.h"
#include "FileUtil.h"

#include <string>
#include <memory>
#include <mutex>

namespace muduozdh{

class LogFile : noncopyable{

public:

    LogFile(const std::string& basename, off_t rollSize ,int flushInterval = 3, int checkEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();

private:

    void append_unlocked(const char* logline, int len);

    static std::string getLogFileName(const std::string& basename, time_t* now);

    const std::string basename_;

    int count_;
    std::mutex mutex_;
    std::unique_ptr<AppendFile> file_;

    const off_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;

    const static int kRollPerSeconds_ = 60*60*24;

};


}
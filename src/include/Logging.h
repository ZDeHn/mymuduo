#pragma once

#include "LogStream.h"
#include "Timestamp.h"

#include <thread>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

namespace muduozdh{

class Logger{

class AsyncLogging;

public:
    Logger(const char *filename, int line);
    ~Logger();

    LogStream &stream() { return impl_.stream_; }

    static void setLogFileName(std::string fileName) { logFileName_ = fileName; }
    static std::string getLogFileName() { return logFileName_; }

private:

    class Impl{
        public:
        Impl(const char *filename, int line);
        void formatTime();

        LogStream stream_;
        int line_;
        std::string basename_;

    };

    Impl impl_;
    static std::string logFileName_;

};
    

}

#define LOG muduozdh::Logger(__FILE__, __LINE__).stream()
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

    enum LogLevel{
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };

    Logger(const char *filename, int line,LogLevel logLevel);
    ~Logger();

    LogStream &stream() { return impl_.stream_; }

    static void setLogFileName(std::string fileName) { logFileName_ = fileName; }
    static std::string getLogFileName() { return logFileName_; }

    static void setGLogLevel(LogLevel logLevel) { gLogLevel = logLevel; }

    static LogLevel getGLogLevel(){
        return gLogLevel;
    }

private:

    class Impl{
        public:
        Impl(const char *filename, int line, LogLevel logLevel);
        void formatTime();

        LogStream stream_;
        int line_;
        std::string basename_;
        LogLevel level_;

    };

    Impl impl_;
    static std::string logFileName_;

    static LogLevel gLogLevel;

};
    

}

#define LOG_TRACE if (muduozdh::Logger::getGLogLevel() <= muduozdh::Logger::TRACE) muduozdh::Logger(__FILE__, __LINE__, muduozdh::Logger::TRACE).stream()
#define LOG_DEBUG if (muduozdh::Logger::getGLogLevel() <= muduozdh::Logger::DEBUG) muduozdh::Logger(__FILE__, __LINE__, muduozdh::Logger::DEBUG).stream()
#define LOG_INFO if (muduozdh::Logger::getGLogLevel() <= muduozdh::Logger::INFO) muduozdh::Logger(__FILE__, __LINE__, muduozdh::Logger::INFO).stream()
#define LOG_WARN if (muduozdh::Logger::getGLogLevel() <= muduozdh::Logger::WARN) muduozdh::Logger(__FILE__, __LINE__, muduozdh::Logger::WARN).stream()
#define LOG_ERROR if (muduozdh::Logger::getGLogLevel() <= muduozdh::Logger::ERROR) muduozdh::Logger(__FILE__, __LINE__, muduozdh::Logger::ERROR).stream()
#define LOG_FATAL if (muduozdh::Logger::getGLogLevel() <= muduozdh::Logger::FATAL) muduozdh::Logger(__FILE__, __LINE__, muduozdh::Logger::FATAL).stream()
#define LOG if (muduozdh::Logger::getGLogLevel() <= muduozdh::Logger::INFO) muduozdh::Logger(__FILE__, __LINE__, muduozdh::Logger::INFO).stream()
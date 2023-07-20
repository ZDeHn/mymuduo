#include "LogFile.h"

#include <iostream>

using namespace muduozdh;

LogFile::LogFile(const std::string& basename, off_t rollSize ,int flushInterval, int checkEveryN)
    : basename_(basename), rollSize_(rollSize), flushInterval_(flushInterval), checkEveryN_(checkEveryN), count_(0), startOfPeriod_(0), lastRoll_(0), lastFlush_(0) {

    rollFile();

}

LogFile::~LogFile() {}

bool LogFile::rollFile(){

    time_t now = 0;
    std::string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_){

        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename));
        return true;
    }

    return false;

}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now){

    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    localtime_r(now, &tm); 
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
    filename += timebuf;

    filename += "log";

    return filename;

}


void LogFile::append(const char* logline, int len){

    std::unique_lock<std::mutex> lock(mutex_);
    append_unlocked(logline, len);
}

void LogFile::flush(){

    std::unique_lock<std::mutex> lock(mutex_);
    file_->flush();
}

void LogFile::append_unlocked(const char* logline, int len){

    file_->append(logline, len);

    if (file_->writtenBytes() > rollSize_){

        rollFile();
    }
    else{

        ++count_;
        if (count_ >= checkEveryN_){

            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;

            if (thisPeriod_ != startOfPeriod_){
                rollFile();
            }
            else if (now - lastFlush_ > flushInterval_)
            {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}
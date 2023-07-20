#pragma once

#include "noncopyable.h"

#include <string>

namespace muduozdh{

class AppendFile : noncopyable{

public:

    explicit AppendFile(std::string filename);
    ~AppendFile();

    void append(const char *logline, const size_t len);
    void flush();

    off_t writtenBytes() const { return writtenBytes_; }

private:

    size_t write(const char *logfile, size_t len);

    FILE *fp_;
    char buffer_[64 * 1024];

    off_t writtenBytes_;

};

}
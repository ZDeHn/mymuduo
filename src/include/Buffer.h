#pragma once

#include <vector>
#include <string>
#include <algorithm>

namespace muduozdh{

class Buffer
{
public:

    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend){}

    size_t readableBytes() const {
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const{
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const{
        return readerIndex_;
    }

    const char* peek() const{
        return begin() + readerIndex_;
    }

    void retrieveUntil(const char *end){
        retrieve(end - peek());
    }

    void retrieve(size_t len){

        if (len < readableBytes()){
            readerIndex_ += len; 
        }
        else {
            retrieveAll();
        }
    }

    void retrieveAll(){

        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    std::string retrieveAllAsString(){

        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len){

        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void ensureWriteableBytes(size_t len){

        if (writableBytes() < len){

            makeSpace(len); 
        }
    }

    void append(const std::string &str){
        append(str.data(), str.size());
    }

    void append(const char *data, size_t len){

        ensureWriteableBytes(len);
        std::copy(data, data+len, beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite(){

        return begin() + writerIndex_;
    }

    const char* beginWrite() const{

        return begin() + writerIndex_;
    }

    const char* findCRLF() const
    {
        const char kCRLF[] = "\r\n";
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }



    ssize_t readFd(int fd, int* saveErrno);
    ssize_t writeFd(int fd, int* saveErrno);

private:
    
    char* begin(){

        return &*buffer_.begin(); 
    }

    const char* begin() const{

        return &*buffer_.begin();
    }
    void makeSpace(size_t len){

        if (writableBytes() + prependableBytes() < len + kCheapPrepend){
            buffer_.resize(writerIndex_ + len);
        }

        else{

            size_t readalbe = readableBytes();
            std::copy(begin() + readerIndex_, 
                    begin() + writerIndex_,
                    begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readalbe;

        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

};

}
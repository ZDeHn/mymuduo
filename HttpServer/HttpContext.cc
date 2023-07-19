#include "HttpContext.h"

#include <muduozdh/Logging.h>

#include <cstring>
#include <string>



HttpContext::URIState HttpContext::processRequestLine(const char *begin, const char *end){

    URIState state = PARSE_URI_ERROR;
    bool succeed;
    const char *start = begin;
    const char *space = std::find(start, end , ' ');

    if(space != end && request_.setMethod(start, space)){

        start = space + 1;
        space = std::find(start, end, ' ');

        if(space != end){

            const char *question = std::find(start, space, '?');
            if(question != space){

                if(*start == '/'){
                    request_.setPath(start + 1, question);
                }
                else{
                    request_.setPath(start, question);
                }

                request_.setQuery(question, space);
            }
            else{
                if(*start == '/'){
                    request_.setPath(start + 1, question);
                }
                else{
                    request_.setPath(start, question);
                }
            }


            
            start = space + 1;
            succeed = (end - start == 8 && std::equal(start, end - 1, "HTTP/1."));

            if (succeed){
                if (*(end-1) == '1'){
                    request_.setVersion(HttpRequest::kHttp11);
                }
                else if (*(end-1) == '0'){
                    request_.setVersion(HttpRequest::kHttp10);
                }
                else{
                    succeed = false;
                }
            }
        }   
    }
    if(succeed){
        return PARSE_URI_SUCCESS;
    }
    else {
        
        return PARSE_URI_ERROR;
    }
}

HttpContext::HeaderState HttpContext::processHeader(muduozdh::Buffer *buf){

    std::string currentBuf(buf->peek(), buf->readableBytes());

    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true; 
    size_t i = 0;

    for(; i < currentBuf.size() && notFinish; ++i){
        switch (hState_) {
        case H_START: {
            if (currentBuf[i] == '\n' || currentBuf[i] == '\r') break;
            hState_ = H_KEY;
            key_start = i;
            now_read_line_begin = i;
            break;
        }
        case H_KEY: {
            if (currentBuf[i] == ':') {
            key_end = i;
            if (key_end - key_start <= 0) return PARSE_HEADER_ERROR;
            hState_ = H_COLON;
            } else if (currentBuf[i] == '\n' || currentBuf[i] == '\r')
            return PARSE_HEADER_ERROR;
            break;
        }
        case H_COLON: {
            if (currentBuf[i] == ' ') {
            hState_ = H_SPACES_AFTER_COLON;
            } else
            return PARSE_HEADER_ERROR;
            break;
        }
        case H_SPACES_AFTER_COLON: {
            hState_ = H_VALUE;
            value_start = i;
            break;
        }
        case H_VALUE: {
            if (currentBuf[i] == '\r') {
                hState_ = H_CR;
                value_end = i;
            if (value_end - value_start <= 0) return PARSE_HEADER_ERROR;
            } else if (i - value_start > 255)
                return PARSE_HEADER_ERROR;
            break;
        }
        case H_CR: {
            if (currentBuf[i] == '\n') {
                hState_ = H_LF;
                std::string key(currentBuf.begin() + key_start, currentBuf.begin() + key_end);
                std::string value(currentBuf.begin() + value_start, currentBuf.begin() + value_end);
                request_.addHeader(key,value);

                // add for debug
                LOG << "http headers insert key: " << key << " value: " << value;

                now_read_line_begin = i;
                buf->retrieveUntil(buf->peek() + i + 1);
            } else
                return PARSE_HEADER_ERROR;
            break;
        }
        case H_LF: {
            if (currentBuf[i] == '\r') {
                hState_ = H_END_CR;
            } else {
                key_start = i;
                hState_ = H_KEY;
            }
            break;
        }
        case H_END_CR: {
            if (currentBuf[i] == '\n') {
                hState_ = H_END_LF;
                buf->retrieveUntil(buf->peek() + i + 1);
            } else
                return PARSE_HEADER_ERROR;
            break;
        }
        case H_END_LF: {
            notFinish = false;
            key_start = i;
            now_read_line_begin = i;
            break;
        }
        }

    }

    if (hState_ == H_END_LF) {
        return PARSE_HEADER_SUCCESS;
    }
    return PARSE_HEADER_AGAIN;


}


bool HttpContext::parseRequest(muduozdh::Buffer* buf, muduozdh::Timestamp receiveTime ){

    // bool ok = false;
    bool hasMore = true;
    bool error = false;

    LOG << "Parse Request";

    while(hasMore){

        if(state_ == kExpectRequestLine){
            LOG << "get state";
            const char* crlf = buf->findCRLF();
            if(crlf){
                
                URIState uriFlag = processRequestLine(buf->peek(), crlf);
                if(uriFlag != PARSE_URI_ERROR){
                    buf->retrieveUntil(crlf + 2);
                    state_ = kExpectHeaders;
                }
                else{
                    hasMore = false;
                    error = true;
                }
            }
            else{
                hasMore = false;
            }
        }

        if(state_ == kExpectHeaders){

            HeaderState headerFlag = processHeader(buf);
            if(headerFlag == PARSE_HEADER_ERROR){
                hasMore = false;
                error = true;
            }
            else if(headerFlag == PARSE_HEADER_AGAIN){
                hasMore = false;
            }
            else if(headerFlag == PARSE_HEADER_SUCCESS){
                if(request_.method() == HttpRequest::kPost){
                    state_ = kExpectBody;
                }
                else{
                    state_ = kGotAll;
                    hasMore = false;
                }
            }
        }
        if(state_ == kExpectBody){

            int contentLength = -1;
            if (request_.getHeader("Content-length") != std::string()) {
                contentLength = stoi(request_.getHeader("Content-length"));
            } else {
                error = true;
                hasMore = false;
            }


            if(static_cast<int>(buf->readableBytes()) > contentLength){

                request_.setBody(std::string(buf->peek(),contentLength));
                buf->retrieveUntil(buf->peek() + contentLength);
                state_ = kGotAll;
                hasMore = false;
            }
            else{
                hasMore = false;
            }

        }

    }

    return !error;

}


HttpContext::URIState HttpContext::processRequestLine(muduozdh::Buffer *buf){

    std::string currentBuf(buf->peek(),buf->readableBytes());


}
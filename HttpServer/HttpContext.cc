#include "HttpContext.h"

#include "Buffer.h"
#include "Logging.h"

HttpContext::LineState HttpContext::ParseRequestLine(const char *begin, const char *end){

    LineState state = PARSE_LINE_ERROR;
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
                    request_.setVersion(HttpRequest::HTTP11);
                }
                else if (*(end-1) == '0'){
                    request_.setVersion(HttpRequest::HTTP10);
                }
                else{
                    succeed = false;
                }
            }
        }   
    }
    if(succeed){
        return PARSE_LINE_SUCCESS;
    }
    else {
        
        return PARSE_LINE_ERROR;
    }
}

bool HttpContext::parseRequest(muduozdh::Buffer* buf, muduozdh::Timestamp receiveTime){

    bool ok = true;
    bool hasMore = true;

    while(hasMore){

        if(state_ == STATE_PARSE_LINE){
            
            const char* crlf = buf->findCRLF();
            if(crlf){
                ok = ParseRequestLine(buf->peek(), crlf) == PARSE_LINE_SUCCESS? true: false ;
                if(ok){
                    request_.setReciveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2);
                    state_ = STATE_PARSE_HEADERS;
                }
                else{
                    hasMore = false;
                }
            }
        }
        else if(state_ == STATE_PARSE_HEADERS){
            
            LOG_DEBUG << "parse headers";
            const char* crlf = buf->findCRLF();
            if(crlf){
                const char* colon = std::find(buf->peek(), crlf, ':');
                if(colon != crlf){
                    request_.addHeader(buf->peek(), colon, crlf);
                    buf->retrieveUntil(crlf + 2);
                }
                else{
                    if(crlf == buf->peek()){
                        state_ = STATE_PARSE_BODY;
                        buf->retrieveUntil(crlf + 2);
                    }
                    else{
                        ok = false;
                        hasMore = false;
                    }
                }
            }

        }
        else if(state_ == STATE_PARSE_BODY){

            LOG_DEBUG << "parse_body " << buf->readableBytes();

            if(request_.method() != HttpRequest::POST || request_.getHeader("Content-Length").empty()){
                state_ = STATE_GET_ALL;
                hasMore = false;
            }
            else{
                int contentLength = stoi(request_.getHeader("Content-length"));

                if(static_cast<int>(buf->readableBytes()) >= contentLength) {
                    request_.setBody(std::string(buf->peek(), contentLength));

                    buf->retrieveUntil(buf->peek() + contentLength);
                    state_ = STATE_GET_ALL;
                    hasMore = false;
                }
                else{
                    hasMore = false;
                }

            }

        }

    }
    
    LOG_DEBUG << "state" << state_;
    return ok;


}
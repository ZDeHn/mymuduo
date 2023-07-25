#pragma once

#include "HttpRequest.h"
#include "Buffer.h"
#include "Timestamp.h"

class HttpContext{


public:

    enum HttpRequestParseState{
        STATE_PARSE_LINE,
        STATE_PARSE_HEADERS,
        STATE_PARSE_BODY,
        STATE_GET_ALL,
    };

    enum LineState{
        PARSE_LINE_SUCCESS,
        PARSE_LINE_AGAIN,
        PARSE_LINE_ERROR,
    };

    enum HeaderState{
        PARSE_HEADER_SUCCESS,
        PARSE_HEADER_AGAIN,
        PARSE_HEADER_ERROR,
    };

    HttpContext(): state_(STATE_PARSE_LINE) { }

    bool parseRequest(muduozdh::Buffer* buf, muduozdh::Timestamp receiveTime);

    bool gotAll() { return state_ == STATE_GET_ALL; }

    void reset(){
        state_ = STATE_PARSE_LINE;
        HttpRequest newRequest;
        request_.swap(newRequest);
    }

    const HttpRequest& request() const{ return request_; }
    HttpRequest& request() { return request_; }

    
private:

    LineState ParseRequestLine(const char* begin, const char* end);

    HttpRequestParseState state_;
    HttpRequest request_;

    
};

#pragma once

#include "HttpRequest.h"

#include <muduozdh/Buffer.h>
#include <muduozdh/Timestamp.h>

#include <string>

class HttpContext{

public:


    enum ProcessState {
        STATE_PARSE_URI = 1,
        STATE_PARSE_HEADERS,
        STATE_RECV_BODY,
        STATE_ANALYSIS,
        STATE_FINISH
    };

    enum URIState {
        PARSE_URI_AGAIN = 1,
        PARSE_URI_ERROR,
        PARSE_URI_SUCCESS,
    };

    enum HeaderState {
        PARSE_HEADER_SUCCESS = 1,
        PARSE_HEADER_AGAIN,
        PARSE_HEADER_ERROR
    };

    enum AnalysisState { ANALYSIS_SUCCESS = 1, ANALYSIS_ERROR };

    enum HeaderParseState {
        H_START = 0,
        H_KEY,
        H_COLON,
        H_SPACES_AFTER_COLON,
        H_VALUE,
        H_CR,
        H_LF,
        H_END_CR,
        H_END_LF
    };

    enum ConnectionState { H_CONNECTED = 0, H_DISCONNECTING, H_DISCONNECTED };

    enum HttpMethod { METHOD_POST = 1, METHOD_GET, METHOD_HEAD };

    enum HttpVersion { HTTP_10 = 1, HTTP_11 };

    enum HttpRequestParseState{
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
        kError
    };

HttpContext()
    :state_(kExpectRequestLine) , hState_(H_START){ } 

bool parseRequest(muduozdh::Buffer *buf, muduozdh::Timestamp receiveTime);

bool gotAll() const { return state_ == kGotAll; }

void reset() {

    state_ = kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);

}

const HttpRequest& request() const { return request_; }
HttpRequest &request() { return request_; }

private:

    URIState processRequestLine(muduozdh::Buffer *buf);
    URIState processRequestLine(const char *begin, const char *end);
    HttpRequestParseState state_;
    HttpRequest request_;

    HeaderState processHeader(muduozdh::Buffer *buf);
    
    HeaderParseState hState_;

};
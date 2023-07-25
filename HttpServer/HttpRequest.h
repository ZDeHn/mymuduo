#pragma once

#include "Timestamp.h"

#include <unordered_map>
#include <string>

class HttpRequest{

public:

    enum Method{
        INVALID,
        GET,
        POST,
        HEAD,
        PUT,
        DELETE,
    };

    enum Version{
        UNKNOWN,
        HTTP10,
        HTTP11,
    };

    HttpRequest(): method_(INVALID), version_(UNKNOWN) { }


    void setVersion(Version version) { version_ = version; }
    Version version() const { return version_; }

    bool setMethod(const char* start, const char* end){

        std::string method(start, end);

        if(method == "GET"){
            method_ = GET;
        }
        else if(method == "POST"){
            method_ = POST;
        }
        else if(method == "HEAD"){
            method_ = HEAD;
        }
        else if(method == "PUT"){
            method_ = PUT;
        }
        else if(method == "DELETE"){
            method_ = DELETE;
        }
        else{
            method = INVALID;
        }
        
        return method_ != INVALID;

    }

    Method method() const { return method_; }

    const char* methodString() const {

        const char* result = "UNKOWN";
        switch (method_)
        {
        case GET:
            result = "GET";
            break;
        case POST:
            result = "POST";
        case HEAD:
            result = "HEAD";
            break;
        case PUT:
            result = "PUT";
        case DELETE:
            result = "DELETE";
            break;
        default:
            break;
        }
        return result;
    }


    void setPath(const char* start, const char* end){
        if(start != end && *start == '/'){
            path_.assign(start+1,end);
        }
        else{
            path_.assign(start,end);
        }
    }
    const std::string& path() const { return path_; }

    void setQuery(const char* start, const char* end){
        query_.assign(start, end);
    }
    const std::string& query() const { return query_; }

    void setBody(const std::string& body) { body_ = body; }
    const std::string& body() const{ return body_; }
    
    void setReciveTime(muduozdh::Timestamp time) { receiveTime_ = time; }
    muduozdh::Timestamp receiveTime() const { return receiveTime_; }

    void addHeader(const char* start, const char* colon, const char* end){

        std::string key(start, colon);
        
        ++colon;
        while(colon < end && isspace(*colon)){
            ++colon;
        }
        std::string value(colon, end);

        while(!value.empty() &&isspace(value[value.size() - 1])){
            value.resize(value.size() - 1);
        }

        headers_[key] = value;

    }
    std::string getHeader(const std::string& key) const{
        std::string result;

        auto it = headers_.find(key);
        if(it != headers_.end()){
            result = it->second;
        }
        return result;
    }

    const std::unordered_map<std::string, std::string> headers() const { return headers_; }

    void swap(HttpRequest& that){
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        receiveTime_.swap(that.receiveTime_);
        headers_.swap(that.headers_);
    }

private:

    Method method_;
    Version version_;

    std::string path_;
    std::string query_;
    std::string body_;

    muduozdh::Timestamp receiveTime_;

    std::unordered_map<std::string, std::string> headers_;

};



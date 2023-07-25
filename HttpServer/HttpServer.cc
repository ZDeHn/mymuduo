#include "HttpServer.h"
#include "HttpResponse.h"
#include "HttpContext.h"
#include "MimeType.h"

#include "Logging.h"
#include "MemoryPool.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <functional>
#include <memory>
#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <map>


const std::string HttpServer::resourcePath = "resource/";

void testRequest(const muduozdh::TcpConnectionPtr& conn, const HttpRequest& req);

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp){

    resp->setStatusCode(HttpResponse::_404NOTFOUND);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

void trim(std::string &srcBuf){

    int spaceIndex;
    
    spaceIndex = srcBuf.find_first_not_of(' ');
    if(spaceIndex != -1){
        srcBuf = srcBuf.substr(spaceIndex, srcBuf.size() - spaceIndex);
    }

    spaceIndex = srcBuf.find_last_not_of(' ');
    if(spaceIndex != -1){
        srcBuf = srcBuf.substr(0,spaceIndex + 1);
    }
    
}



HttpServer::HttpServer(muduozdh::EventLoop *loop,const muduozdh::InetAddress &listenAddr, const std::string &name, muduozdh::TcpServer::Option option)
    : server_(loop, listenAddr, name, option), httpCallback_(defaultHttpCallback){

    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    server_.setThreadNum(4);
}

void HttpServer::start(){
    
    LOG_INFO <<"HttpServer[" << server_.name() << "] starts listening on " << server_.ipPort();
    server_.start();
}

void HttpServer::onConnection(const muduozdh::TcpConnectionPtr& conn){

    if(conn->connected()){
        LOG_INFO << "new Connection arrived";
        // std::shared_ptr<HttpContext> context(new HttpContext());
        std::shared_ptr<HttpContext> context(httpContextMemoryPool_.newData(),[&](HttpContext* context){
            httpContextMemoryPool_.deleteData(context);
        });
        conn->setContext(context);
    }
    else{
        LOG_INFO << "Connection closed";
    }
}



void HttpServer::onMessage(const muduozdh::TcpConnectionPtr& conn, muduozdh::Buffer* buf, muduozdh::Timestamp receiveTime){


    
    HttpContext* context = (HttpContext *)(conn->context().get());

    LOG_DEBUG << "get Context";

    if(!context->parseRequest(buf, receiveTime)){

        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        LOG_INFO << "request error";
        conn->shutdown();
    }

    if(context->gotAll()){
        LOG_INFO << "parseRequest success!";
        // onRequest(conn, context->request());
        testRequest(conn, context->request());
        context->reset();
    }

}



void HttpServer::onRequest(const muduozdh::TcpConnectionPtr& conn, const HttpRequest& req){


    const std::string& connection = req.getHeader("Connection");
    bool close = connection == "close" ||
        (req.version() == HttpRequest::HTTP10 && connection != "Keep-Alive");

    std::string requestPath = req.path();
    int dotPos = requestPath.find('.');
    std::string filetype;

    if (dotPos < 0)
        filetype = MimeType::getMime("default");
    else{

        std::string fileSuffix = requestPath.substr(dotPos);
        std::transform(fileSuffix.begin() , fileSuffix.end(), fileSuffix.begin(), tolower);

        LOG_DEBUG << fileSuffix;
        
        filetype = MimeType::getMime(fileSuffix);
        
    }

    trim(requestPath);

    if(requestPath.empty()){
        requestPath = "index.html";
    }

    requestPath = resourcePath + requestPath;

    HttpResponse response(close);
    muduozdh::Buffer responseBuffer;    
    std::string body;    

    struct stat sbuf;

    if(stat(requestPath.c_str(), &sbuf) < 0) {
        
        LOG_ERROR << "file: " << requestPath << " not found";
        defaultHttpCallback(req, &response);
        response.appendToBuffer(&responseBuffer);
        conn->send(&responseBuffer);
        if(response.closeConnection()){
            conn->shutdown();
        }
        return ;
    }
    
    else{

        response.setContentType(filetype);
        response.addHeader("Server","Tiny Web Server");

        int srcFd = ::open(requestPath.c_str(), O_RDONLY, 0);
        if(srcFd < 0){
 
            LOG_ERROR << "file: " << requestPath << " not found";
            defaultHttpCallback(req, &response);
            response.appendToBuffer(&responseBuffer);
            conn->send(&responseBuffer);
            if(response.closeConnection()){
                conn->shutdown();
            }
            return ;
        }

        void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
        ::close(srcFd);
        
        if (mmapRet == (void *)-1) {

            munmap(mmapRet, sbuf.st_size);

            LOG_ERROR << "file: " << requestPath << " map error!";
            defaultHttpCallback(req, &response);
            response.appendToBuffer(&responseBuffer);
            conn->send(&responseBuffer);

            if(response.closeConnection()){
                conn->shutdown();
            }
            return ;
        }
        
        char *srcAddr = static_cast<char *>(mmapRet);
        std::string file(srcAddr, srcAddr + sbuf.st_size);
        munmap(mmapRet, sbuf.st_size);
        body = file;
    
    }
    response.setBody(body);
    response.appendToBuffer(&responseBuffer);
    conn->send(&responseBuffer);
    if(response.closeConnection()){
        conn->shutdown();
    }
}

void testRequest(const muduozdh::TcpConnectionPtr& conn, const HttpRequest& req){

    LOG_DEBUG << req.path();

    const std::string& connection = req.getHeader("Connection");

    bool close = connection == "close" ||
        (req.version() == HttpRequest::HTTP10 && connection != "Keep-Alive");

    HttpResponse response(close);
    response.setContentType("text/html");
    response.addHeader("Server","HTTP Server");
    std::string body = "Hello";
    response.setBody(body);

    muduozdh::Buffer responseBuffer; 

    response.appendToBuffer(&responseBuffer);
        
    conn->send(&responseBuffer);

    if(response.closeConnection()){
        conn->shutdown();
    }
    return ;
    
}

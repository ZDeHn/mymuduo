#pragma once

#include "HttpResponse.h"
#include "HttpContext.h"

#include <muduozdh/TcpServer.h>

#include <string>

class HttpServer{

public:

    using HttpCallback = std::function<void (const HttpRequest&, HttpResponse*)>;

    HttpServer(muduozdh::EventLoop *loop, const muduozdh::InetAddress& listenAddr, const std::string& name, muduozdh::TcpServer::Option option = muduozdh::TcpServer::kNoReusePort);

    muduozdh::EventLoop* getLoop() const { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& cb){
        httpCallback_ = cb;
    }

    void start();

private:

    void onConnection(const muduozdh::TcpConnectionPtr& conn);
    void onMessage(const muduozdh::TcpConnectionPtr &con, muduozdh::Buffer *buf, muduozdh::Timestamp receiveTime);
    void onRequest(const muduozdh::TcpConnectionPtr&, const HttpRequest&);

    muduozdh::TcpServer server_;
    HttpCallback httpCallback_;

    const static std::string resourcePath;
    
};
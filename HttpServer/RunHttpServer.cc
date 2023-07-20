#include "HttpServer.h"

#include "Logging.h"
#include "EventLoop.h"

int main(){

    muduozdh::Logger::setLogFileName("./HttpServer");

    muduozdh::EventLoop loop;
    muduozdh::InetAddress addr(80);

    HttpServer httpServer(&loop, addr, "httpServer", muduozdh::TcpServer::kReusePort);


    httpServer.start();
    loop.loop();

}
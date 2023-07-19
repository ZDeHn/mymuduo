#include "HttpServer.h"

#include <muduozdh/Logging.h>
#include <muduozdh/EventLoop.h>


int main(){

    muduozdh::EventLoop loop;
    muduozdh::InetAddress addr(80);

    HttpServer httpServer(&loop, addr, "httpServer", muduozdh::TcpServer::kReusePort);

    muduozdh::Logger::setLogFileName("./HttpServer.log");

    httpServer.start();
    loop.loop();

}
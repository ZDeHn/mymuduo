#include "HttpServer.h"

#include "Logging.h"
#include "EventLoop.h"
#include "MemoryPool.h"

int main(){

    muduozdh::Logger::setLogFileName("./HttpServer");
    muduozdh::Logger::setGLogLevel(muduozdh::Logger::DEBUG);

    muduozdh::EventLoop loop;
    muduozdh::InetAddress addr(80);

    HttpServer httpServer(&loop, addr, "httpServer", muduozdh::TcpServer::kReusePort);


    httpServer.start();
    loop.loop();

}
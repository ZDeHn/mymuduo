#include "TcpServer.h"
#include "Logging.h"
#include "TcpConnection.h"

#include <strings.h>
#include <functional>

using namespace muduozdh;

static EventLoop* CheckLoopNotNull(EventLoop *loop){

    if (loop == nullptr){

        LOG_FATAL << "mainLoop is null";
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop,const InetAddress &listenAddr,const std::string &nameArg,Option option)
    : loop_(CheckLoopNotNull(loop)), ipPort_(listenAddr.toIpPort()), name_(nameArg), acceptor_(new Acceptor(loop, listenAddr, option == kReusePort))
    , threadPool_(new EventLoopThreadPool(loop, name_)), connectionCallback_(), messageCallback_(), nextConnId_(1), started_(0){

    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));

}

TcpServer::~TcpServer(){

    for (auto &item : connections_){

        TcpConnectionPtr conn(item.second); 
        item.second.reset();

        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads){

    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start(){

    if (started_++ == 0) {
        threadPool_->start(threadInitCallback_); 
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr){

    EventLoop *ioLoop = threadPool_->getNextLoop(); 
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection ["<< name_ << "] - new conncetion [" << connName << "] from " << peerAddr.toIpPort(); 

    sockaddr_in local;
    ::bzero(&local, sizeof local);
    socklen_t addrlen = sizeof local;
    if (::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0){

        LOG_INFO << "sockets::getLocalAddr";
    }

    InetAddress localAddr(local);

    // TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,sockfd,localAddr,peerAddr));
    TcpConnectionPtr conn(connectionMemoryPool_.newData(ioLoop, connName, sockfd, localAddr, peerAddr), [&](TcpConnection* connection){
        connectionMemoryPool_.deleteData(connection);
    });

    connections_[connName] = conn;

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn){

    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn){

    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection " << conn->name();

    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop(); 
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));

}
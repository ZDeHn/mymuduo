#include "Acceptor.h"
#include "Logging.h"
#include "InetAddress.h"

#include <sys/types.h>    
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

using namespace muduozdh;

static int createNonblocking(){

    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG << "listen socket create error : " << errno;
    }
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    : loop_(loop), acceptSocket_(createNonblocking()), acceptChannel_(loop, acceptSocket_.fd()), listenning_(false){

    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);

    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor(){

    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen(){

    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();

}

void Acceptor::handleRead(){

    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);

    if (connfd >= 0){

        if (newConnectionCallback_){

            newConnectionCallback_(connfd, peerAddr);
        }

        else{
            ::close(connfd);
        }
    }
    else{

        LOG << "accept error " << errno;

        if (errno == EMFILE){
            LOG << "sockfd reached limit ";
        }
    }
}
#include "Acceptor.h"
#include "Logging.h"
#include "InetAddress.h"

#include <sys/types.h>    
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

using namespace muduozdh;

static int createNonblocking(){

    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG_ERROR << "listen socket create error : " << errno;
    }
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    : loop_(loop), acceptSocket_(createNonblocking()), acceptChannel_(loop, acceptSocket_.fd()), listenning_(false)
    , idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)){

    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);

    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor(){

    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
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

        LOG_ERROR << "accept error " << errno;

        if (errno == EMFILE){
            LOG_ERROR << "sockfd reached limit ";
            ::close(idleFd_);
            ::accept(idleFd_, nullptr, nullptr);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
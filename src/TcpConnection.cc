#include "TcpConnection.h"
#include "Logging.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>
#include <errno.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <string>

using namespace muduozdh;

static EventLoop* CheckLoopNotNull(EventLoop *loop){

    if (loop == nullptr){

        LOG << "TcpConnection Loop is null！" ;
        exit(1);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &nameArg, int sockfd,const InetAddress& localAddr,const InetAddress& peerAddr)
    : loop_(CheckLoopNotNull(loop)), name_(nameArg), state_(kConnecting), reading_(true), socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd)), localAddr_(localAddr), peerAddr_(peerAddr), highWaterMark_(64*1024*1024){

    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)
    );

    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this)
    );

    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );

    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this)
    );

    LOG << "TcpConnection :: ctor[" << name_ << "] at fd =" << sockfd;
    
    socket_->setKeepAlive(true);
}


TcpConnection::~TcpConnection(){

    LOG << "TcpConnction::dtor[" << name_<< "] at fd = " << channel_->fd() << " state = " << (int)state_;
}

void TcpConnection::send(const std::string &buf){

    if (state_ == kConnected){
        
        if (loop_->isInLoopThread()){

            sendInLoop(buf.c_str(), buf.size());
        }
        else{

            void(TcpConnection::*fp)(const void* data, size_t len) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::send(Buffer *buf)
{
    if (state_ == kConnected){

        if (loop_->isInLoopThread()){
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        
        else{
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
        }
    }
}


void TcpConnection::sendInLoop(const void* data, size_t len){
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (state_ == kDisconnected){

        LOG << "disconnected, give up writing !";
        return;
    }

    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0){

        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0){

            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_){

                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this())
                );
            }
        }
        else{

            nwrote = 0;
            if (errno != EWOULDBLOCK){

                LOG << "TcpConnection::sendInLoop";

                if (errno == EPIPE || errno == ECONNRESET){
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && remaining > 0){

        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_){

            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen+remaining));

        }
        outputBuffer_.append((char*)data + nwrote, remaining);
        
        if (!channel_->isWriting()){
            channel_->enableWriting();
        }
    }
}


void TcpConnection::sendInLoop(const std::string& message){

    sendInLoop(message.data(), message.size());
}


void TcpConnection::shutdown(){

    if (state_ == kConnected){
    
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));

    }
}

void TcpConnection::shutdownInLoop(){

    if (!channel_->isWriting()) {

        socket_->shutdownWrite(); 
    }
}

void TcpConnection::connectEstablished(){

    setState(kConnected);

    channel_->tie(shared_from_this());
    channel_->enableReading(); 

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed(){

    if (state_ == kConnected){

        setState(kDisconnected);
        channel_->disableAll(); 
        connectionCallback_(shared_from_this());
    }

    channel_->remove(); 
}

void TcpConnection::handleRead(Timestamp receiveTime){

    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);

    if (n > 0){

        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0){

        handleClose();
    }
    else{

        errno = savedErrno;

        LOG << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting()){

        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
        
        if (n > 0){

            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0){

                channel_->disableWriting();
                if (writeCompleteCallback_){

                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting){
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG << "TcpConnection::handleWrite";
        }
    }
    else{
        LOG << "TcpCOnnection fd = " <<  channel_->fd() << " is down , on more writing";
    }
}

void TcpConnection::handleClose(){

    LOG << "TcpConnection::handleClose fd = " << channel_->fd() << " state = " << (int)state_ ;
    
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());

    connectionCallback_(connPtr);
    closeCallback_(connPtr); 
}

void TcpConnection::handleError(){

    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0){

        err = errno;
    }
    else{

        err = optval;
    }

    LOG << "TcpConnection::handleErro name: " << name_ << "- SO_ERROR" << err; 

}
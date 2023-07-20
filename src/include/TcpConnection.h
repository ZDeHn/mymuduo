#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <atomic>

// #include <boost/any.hpp>

namespace muduozdh{

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, 
                const std::string &name, 
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    void send(const std::string &buf);
    void send(Buffer *buf);

    void shutdown();

    void forceClose();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark){ 
        highWaterMarkCallback_ = cb; 
        highWaterMark_ = highWaterMark; 
    }

    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    void connectEstablished();
    void connectDestroyed();

    // const boost::any& context() const { return context_; }
    // void setContext(const boost::any& context) { context_ = context_; }

    const std::shared_ptr<void> context() const { return context_; }
    void setContext(const std::shared_ptr<void> context) { context_ = context; }


private:
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};
    void setState(StateE state) { state_ = state; }

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void sendInLoop(const std::string& message);

    void shutdownInLoop();
    void forceCloseInLoop();

    EventLoop *loop_; 
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_; 
    MessageCallback messageCallback_; 
    WriteCompleteCallback writeCompleteCallback_; 
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;

    Buffer inputBuffer_;  
    Buffer outputBuffer_; 

    std::shared_ptr<void> context_;
    // boost::any context_;

};

}